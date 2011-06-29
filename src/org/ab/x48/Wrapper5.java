package org.ab.x48;

import android.content.Context;
import android.content.pm.PackageManager;
import android.view.MotionEvent;

class Wrapper5 {

	public static final boolean supportsMultitouch(Context context) {
		if (Wrapper.SDK_INT < 7)
			return true;

		return context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_TOUCHSCREEN_MULTITOUCH);
	}

	public static final int MotionEvent_getPointerCount(MotionEvent event) {
		return event.getPointerCount();
	}

	public static final int MotionEvent_getPointerId(MotionEvent event,
			int pointerIndex) {
		return event.getPointerId(pointerIndex);
	}

	public static final int MotionEvent_findPointerIndex(MotionEvent event,
			int pointerId) {
		return event.findPointerIndex(pointerId);
	}

	public static final float MotionEvent_getX(MotionEvent event,
			int pointerIndex) {
		return event.getX(pointerIndex);
	}

	public static final float MotionEvent_getY(MotionEvent event,
			int pointerIndex) {
		return event.getY(pointerIndex);
	}

	public static final float MotionEvent_getSize(MotionEvent event,
			int pointerIndex) {
		return event.getSize(pointerIndex);
	}
}
