#include <jni.h>
#include <android/log.h>

#include <array>
#include <cinttypes>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

namespace {
constexpr const char* kLogTag = "DiagNative";
constexpr std::array<uintptr_t, 4> kOffsets = {0x224, 0x44, 0x94, 0x7C};

struct NativePacket {
    std::string label;
    int32_t state;
    float load;
};

uintptr_t FindLibraryBase(const std::string& soName) {
    std::ifstream maps("/proc/self/maps");
    std::string line;

    while (std::getline(maps, line)) {
        if (line.find(soName) == std::string::npos) {
            continue;
        }

        std::istringstream iss(line);
        std::string range;
        iss >> range;

        const auto dash = range.find('-');
        if (dash == std::string::npos) {
            continue;
        }

        const std::string startHex = range.substr(0, dash);
        return static_cast<uintptr_t>(strtoull(startHex.c_str(), nullptr, 16));
    }

    return 0;
}

template <typename T>
bool ReadMemory(uintptr_t address, T* out) {
    if (address == 0 || out == nullptr) {
        return false;
    }

    memcpy(out, reinterpret_cast<void*>(address), sizeof(T));
    return true;
}

std::array<uint32_t, 4> ReadAnchorValues(uintptr_t anchor) {
    std::array<uint32_t, 4> values = {0, 0, 0, 0};

    for (size_t i = 0; i < kOffsets.size(); ++i) {
        ReadMemory(anchor + kOffsets[i], &values[i]);
    }

    return values;
}

jobject ToJavaPacket(JNIEnv* env, const NativePacket& packet) {
    const jclass cls = env->FindClass("com/example/diagnostics/DataPacket");
    if (cls == nullptr) {
        return nullptr;
    }

    const jmethodID ctor = env->GetMethodID(
        cls,
        "<init>",
        "(Ljava/lang/String;IF)V");
    if (ctor == nullptr) {
        return nullptr;
    }

    jstring label = env->NewStringUTF(packet.label.c_str());
    jobject result = env->NewObject(cls, ctor, label, packet.state, packet.load);
    env->DeleteLocalRef(label);
    return result;
}

NativePacket FromJavaPacket(JNIEnv* env, jobject packetObj) {
    NativePacket out{"", 0, 0.0f};
    if (packetObj == nullptr) {
        return out;
    }

    jclass cls = env->GetObjectClass(packetObj);
    jfieldID labelField = env->GetFieldID(cls, "label", "Ljava/lang/String;");
    jfieldID stateField = env->GetFieldID(cls, "state", "I");
    jfieldID loadField = env->GetFieldID(cls, "load", "F");

    jstring labelObj = static_cast<jstring>(env->GetObjectField(packetObj, labelField));
    const char* labelChars = env->GetStringUTFChars(labelObj, nullptr);
    out.label = labelChars != nullptr ? labelChars : "";
    if (labelChars != nullptr) {
        env->ReleaseStringUTFChars(labelObj, labelChars);
    }

    out.state = env->GetIntField(packetObj, stateField);
    out.load = env->GetFloatField(packetObj, loadField);
    env->DeleteLocalRef(labelObj);

    return out;
}

std::string BuildHudLine(uintptr_t libBase, uintptr_t anchor, const std::array<uint32_t, 4>& values) {
    std::ostringstream oss;
    oss << "lib=0x" << std::hex << libBase
        << " anchor=0x" << anchor
        << std::dec
        << " [0x224=" << values[0]
        << ", 0x44=" << values[1]
        << ", 0x94=" << values[2]
        << ", 0x7C=" << values[3] << "]";
    return oss.str();
}
}  // namespace

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_diagnostics_NativeBridge_getHudSnapshot(
    JNIEnv* env,
    jclass /*clazz*/,
    jstring libraryName,
    jlong pointerAnchor) {

    const char* libChars = env->GetStringUTFChars(libraryName, nullptr);
    std::string libName = libChars != nullptr ? libChars : "libUnityPlayer.so";
    if (libChars != nullptr) {
        env->ReleaseStringUTFChars(libraryName, libChars);
    }

    uintptr_t base = FindLibraryBase(libName);
    uintptr_t anchor = static_cast<uintptr_t>(pointerAnchor);
    auto values = ReadAnchorValues(anchor);
    std::string hud = BuildHudLine(base, anchor, values);
    return env->NewStringUTF(hud.c_str());
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_example_diagnostics_NativeBridge_enrichPacket(
    JNIEnv* env,
    jclass /*clazz*/,
    jobject inPacket) {

    NativePacket packet = FromJavaPacket(env, inPacket);
    packet.label += " (native)";
    packet.state += 1;
    packet.load += 0.125f;
    return ToJavaPacket(env, packet);
}
