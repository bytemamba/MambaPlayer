package com.mamba.player

import android.os.Bundle
import android.os.Environment
import android.util.Log
import android.view.SurfaceView
import android.widget.SeekBar
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import java.io.File

class MainActivity : AppCompatActivity() {
    private lateinit var player: Player
    private var isTouch = false
    private var totalDuration = 0

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        val surface = findViewById<SurfaceView>(R.id.surface)
        val seek = findViewById<SeekBar>(R.id.seekBar)
        val tvTime = findViewById<TextView>(R.id.tvTime)

        seek.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                if (fromUser) {
                    tvTime.text =
                        "${getMinutes(progress)}:${getSeconds(progress)}" +
                                "/${getMinutes(totalDuration)}:${getSeconds(totalDuration)}"
                }
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {
                isTouch = true
            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
                isTouch = false
                player.seekTo(seekBar?.progress?.toLong() ?: 0L)
            }
        })

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
                totalDuration = player.getDuration()
                runOnUiThread {
                    seek.max = totalDuration
                }
            }
        })
        player.setProgressAction {
            if (!isTouch) {
                runOnUiThread {
                    tvTime.text = "${getMinutes(it)}:${getSeconds(it)}" +
                            "/${getMinutes(totalDuration)}:${getSeconds(totalDuration)}"
                    seek.progress = it
                }
            }
        }
    }

    override fun onResume() {
        super.onResume()
        player.prepare()
    }

    override fun onStop() {
        super.onStop()
        player.stop()
    }

    override fun onDestroy() {
        super.onDestroy()
        player.release()
    }

    private fun getMinutes(duration: Int): String {
        val minutes = duration / 60
        return if (minutes <= 9) "0$minutes" else "" + minutes
    }

    private fun getSeconds(duration: Int): String {
        val seconds = duration % 60
        return if (seconds <= 9) "0$seconds" else "" + seconds
    }
}