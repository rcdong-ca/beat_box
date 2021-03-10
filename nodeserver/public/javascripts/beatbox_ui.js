//commands
//volume control: vol- or vol+
//BPM control: bpm+ or bpm-
//Mode change: mode
//Play instrument: bass, snare, hihat


$(document).ready(function() {

    var socket = io()

    var timeout
    var errorBox = document.getElementById("error-box")
    errorBox.style.display = "none"

    window.setInterval( function() {
        console.log("get server time")
        socket.emit("get uptime")
        if (!socket.connected) {
            errorBox.style.display = "block"
            if (timeout)
                clearTimeout(timeout)
        }
        else {
            timeout = setTimeout(function() {
                if (socket.connected) {
                    errorBox.style.display = "none"
                }
            }, 10000)
        }
        

    }, 1000)

    socket.emit("get default", "default")

    $("#volumeUp").click(function() {
        console.log("Volume up Clicked")
        socket.emit("set change", "vol+")
    })

    $("#volumeDown").click(function() { 
        console.log("Volume Down Clicked")
        socket.emit("set change", "vol-")
    })

    $("#bpmUp").click(function() {
        console.log("BPM up Clicked")
        socket.emit("set change", "bpm+")
    })

    $("#bpmDown").click(function() { 
        console.log("BPM Down Clicked")
        socket.emit("set change", "bpm-")
    })

    $("#modeNone").click(function() { 
        console.log("modeNone Clicked")
        socket.emit("set change", "mode:0")
    })
    $("#modeRock1").click(function() { 
        console.log("mode1 Clicked")
        socket.emit("set change", "mode:1")
    })
    $("#modeRock2").click(function() { 
        console.log("mode2 Clicked")
        socket.emit("set change", "mode:2")
    })

    $("#modeNone").click(function() { 
        console.log("modeNone Clicked")
        socket.emit("set change", "mode:0")
    })
    $("#modeRock1").click(function() { 
        console.log("mode1 Clicked")
        socket.emit("set change", "mode:1")
    })
    $("#modeRock2").click(function() { 
        console.log("mode2 Clicked")
        socket.emit("set change", "mode:2")
    })

    $("#hi_hat").click(function() { 
        console.log("hi_hat Clicked")
        socket.emit("set drum", "hi_hat")
    })

    $("#snare").click(function() { 
        console.log("snare Clicked")
        socket.emit("set drum", "snare")
    })

    $("#bass").click(function() { 
        console.log("bass Clicked")
        socket.emit("set drum", "bass")
    })


    socket.on('set volume', function(num) {
        console.log("received" + num)
        var n = parseInt(num)
        $('#volumeid').val( n ) 
    })

    socket.on('set bpm', function(num) {
        console.log("received" + num)
        var n = parseInt(num)
        $('#bpmid').val( n ) 
    })
    socket.on("get default", function(str) {
        console.log("default received ="+str)
        var res = str.split(":")
        console.log("Res0 = " + res[0])
        $('#volumeid').val((res[0]))
        $('#bpmid').val(parseInt(res[1]))
    })

    socket.on("set drum", function() {
        console.log("DRUM SOUND PLAYED")
    })

    socket.on("set mode", function(str) {
        console.log("mode received = "+str)
        var res = str
        var theMode = "None"
        if (res =="1") {
            theMode = "Rock#1"
        }
        else if (res=="2") {
            theMode = "Custom2"
        }
        $("#modeid").html(theMode)
    })

    socket.on('get uptime', function(time) {
        var res = time.split(" ")
        var seconds = parseFloat(res[0])
        var minutes = Math.floor(seconds / 60)
        var hours = Math.floor( minutes/ 60)
        seconds = Math.floor(seconds % 60)
        $('#serverUpTime').html(hours + ":" + minutes + ":"+seconds)
    })
})