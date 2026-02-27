package com.example.diagnostics;

import android.app.Service;
import android.content.Intent;
import android.graphics.Color;
import android.graphics.PixelFormat;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.view.Gravity;
import android.view.WindowManager;
import android.widget.TextView;

public class HudOverlayService extends Service {
    private static final long UPDATE_INTERVAL_MS = 500L;

    private WindowManager windowManager;
    private TextView hudView;
    private Handler handler;

    private final Runnable updateTask = new Runnable() {
        @Override
        public void run() {
            long anchor = 0x0L; // Replace with an anchor pointer supplied by your runtime probe.
            String hudData = NativeBridge.getHudSnapshot("libUnityPlayer.so", anchor);

            DataPacket probe = new DataPacket("sync", 7, 0.32f);
            DataPacket enriched = NativeBridge.enrichPacket(probe);

            String text = hudData
                + "\nlabel=" + enriched.label
                + " state=" + enriched.state
                + " load=" + enriched.load;
            hudView.setText(text);

            handler.postDelayed(this, UPDATE_INTERVAL_MS);
        }
    };

    @Override
    public void onCreate() {
        super.onCreate();
        windowManager = (WindowManager) getSystemService(WINDOW_SERVICE);
        handler = new Handler(Looper.getMainLooper());

        hudView = new TextView(this);
        hudView.setTextColor(Color.WHITE);
        hudView.setTextSize(11f);
        hudView.setBackgroundColor(0x55000000);
        hudView.setPadding(12, 8, 12, 8);

        WindowManager.LayoutParams params = new WindowManager.LayoutParams(
            WindowManager.LayoutParams.WRAP_CONTENT,
            WindowManager.LayoutParams.WRAP_CONTENT,
            WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY,
            WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                | WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL
                | WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN,
            PixelFormat.TRANSLUCENT
        );
        params.gravity = Gravity.TOP | Gravity.START;
        params.x = 12;
        params.y = 48;

        windowManager.addView(hudView, params);
        handler.post(updateTask);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        handler.removeCallbacks(updateTask);
        if (hudView != null) {
            windowManager.removeView(hudView);
            hudView = null;
        }
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
}
