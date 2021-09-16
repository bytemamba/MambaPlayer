package com.mamba.player

import android.os.Bundle
import android.os.Environment
import android.util.Log
import android.view.SurfaceView
import androidx.appcompat.app.AppCompatActivity
import java.io.File

class MainActivity : AppCompatActivity() {
    private lateinit var player: Player

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        val surface = findViewById<SurfaceView>(R.id.surface)

        player = Player(
            File(
                Environment.getExternalStorageDirectory().absolutePath + File.separator + "demo.mp4"
            ).absolutePath
        )
        player.setSurfaceView(surface)
        player.setPlayerListener(object : Player.PlayerListener {
            override fun onPrepared() {
                Log.e("X_TAG", "onPrepared: ")
                player.play()
            }
        })
    }

    override fun onResume() {
        super.onResume()
        player.prepare()
    }
}