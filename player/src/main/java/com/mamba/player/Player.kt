package com.mamba.player

import android.util.Log
import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView

class Player(private val dataSource: String) : SurfaceHolder.Callback {

    companion object {
        init {
            System.loadLibrary("player")
        }
    }

    private var playerListener: PlayerListener? = null
    private var surfaceHolder: SurfaceHolder? = null

    /**
     * 打开文件
     */
    fun prepare() {
        Log.e("X_TAG", "prepare: $dataSource")
        nativePrepare(dataSource)
    }

    /**
     * 开始播放
     */
    fun play() {
        nativePlay()
    }

    /**
     * 停止播放
     */
    fun stop() = nativeStop()

    /**
     * 释放资源
     */
    fun release() = nativeRelease()

    /**
     * 设置播放监听
     *
     * 子线程
     */
    fun setPlayerListener(playerListener: PlayerListener) {
        this.playerListener = playerListener
    }

    /**
     * 绑定 surface 监听
     */
    fun setSurfaceView(surfaceView: SurfaceView) {
        if (surfaceHolder != null) {
            surfaceHolder!!.removeCallback(this);
        }
        surfaceHolder = surfaceView.holder
        surfaceHolder?.addCallback(this)
    }

    // --------- Native methods
    private external fun nativePrepare(dataSource: String)
    private external fun nativePlay()
    private external fun nativeStop()
    private external fun nativeRelease()
    private external fun nativeSetSurface(surface: Surface)

    // --------- JNI callback
    fun onPrepared() {
        Log.e("X_TAG", "JNI callback")
        playerListener?.onPrepared()
    }

    interface PlayerListener {
        /**
         * 初始化成功
         */
        fun onPrepared()
    }

    // -------- Surface callback
    override fun surfaceCreated(holder: SurfaceHolder?) {
    }

    override fun surfaceChanged(holder: SurfaceHolder?, format: Int, width: Int, height: Int) {
        holder?.let {
            nativeSetSurface(it.surface)
        }
    }

    override fun surfaceDestroyed(holder: SurfaceHolder?) {
    }
}