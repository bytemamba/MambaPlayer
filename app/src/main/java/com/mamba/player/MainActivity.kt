package com.mamba.player

import android.os.Bundle
import android.os.Environment
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import java.io.File

class MainActivity : AppCompatActivity() {
    private lateinit var player: Player

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        player = Player(
            File(
                Environment.getExternalStorageDirectory().absolutePath + File.separator + "demo.mov"
            ).absolutePath
        )
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