package com.example.diagnostics;

public class DataPacket {
    public final String label;
    public final int state;
    public final float load;

    public DataPacket(String label, int state, float load) {
        this.label = label;
        this.state = state;
        this.load = load;
    }
}
