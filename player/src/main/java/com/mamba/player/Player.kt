package com.mamba.player

import android.util.Log
import android.view.SurfaceHolder

class Player(private val dataSource: String) {

    companion object {
        init {
            System.loadLibrary("player")
        }
    }

    private var playerListener: PlayerListener? = null

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

    fun setSurfaceHolder(surfaceHolder: SurfaceHolder) {

    }

    // --------- Native methods
    private external fun nativePrepare(dataSource: String)
    private external fun nativePlay()
    private external fun nativeStop()
    private external fun nativeRelease()

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
}