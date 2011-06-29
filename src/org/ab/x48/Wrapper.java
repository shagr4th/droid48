package org.ab.x48;

import android.content.Context;
import android.os.Build;
import android.view.MotionEvent;

public class Wrapper {

	static final int SDK_INT = Integer.parseInt(Build.VERSION.SDK);

	public static boolean supportsMultitouch(Context context) {
		if (SDK_INT >= 5)
			return Wrapper5.supportsMultitouch(context);
		return false;
	}

	public static final int MotionEvent_getPointerCount(MotionEvent event) {
		if (SDK_INT >= 5)
			return Wrapper5.MotionEvent_getPointerCount(event);
		return 1;
	}

	public static final int MotionEvent_getPointerId(MotionEvent event,
			int pointerIndex) {
		if (SDK_INT >= 5)
			return Wrapper5.MotionEvent_getPointerId(event, pointerIndex);
		return 0;
	}

	public static final int MotionEvent_findPointerIndex(MotionEvent event,
			int pointerId) {
		if (SDK_INT >= 5)
			return Wrapper5.MotionEvent_findPointerIndex(event, pointerId);
		if (pointerId == 0)
			return 0;
		return -1;
	}

	public static final float MotionEvent_getX(MotionEvent event,
			int pointerIndex) {
		if (SDK_INT >= 5)
			return Wrapper5.MotionEvent_getX(event, pointerIndex);
		return event.getX();
	}

	public static final float MotionEvent_getY(MotionEvent event,
			int pointerIndex) {
		if (SDK_INT >= 5)
			return Wrapper5.MotionEvent_getY(event, pointerIndex);
		return event.getY();
	}

	public static final float MotionEvent_getSize(MotionEvent event,
			int pointerIndex) {
		if (SDK_INT >= 5)
			return Wrapper5.MotionEvent_getSize(event, pointerIndex);
		return event.getSize();
	}
}
