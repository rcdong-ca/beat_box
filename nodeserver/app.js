
const express = require('express')
const app = express()
const http = require('http').Server(app)
const port = 8088 
const path = require('path')
const io = require('socket.io')(http)
const dgram = require('dgram')
const e = require('express')
const udpSocket = dgram.createSocket('udp4')
const udpPort = 2000
const targetPort = 3000

//emit to client about server up time
var fs = require('fs')
                    
//set static folder
app.use(express.static(path.join(__dirname, 'public')))

http.listen(port, () => {
    console.log('Listening on port:' + port)
})

app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'public/stylesheets/index.html'))
})

io.on('connection', (socket) => {
    console.log('a user connected')
    socket.on('disconnect', () => {
        console.log("user disconnected")
    })

    socket.on("set change", (msg) => {                     //send msg to C app
        udpSocket.send(msg, targetPort, '192.168.7.2', (err) => {
            console.log("sening packet to C app...")
        })
    })

    socket.on("get default", (msg) => {                     //send msg to C app
        udpSocket.send(msg, targetPort, '192.168.7.2', (err) => {
            console.log("sening default packet to C app...")
        })
    })

    socket.on('set drum', (msg) => {
        udpSocket.send(msg, targetPort, '192.168.7.2', (err) => {
            console.log("sending drum packet to C app")
        })
    })

    socket.on('get uptime', () => {
        fs.readFile('/proc/uptime', function(err, data) {
            console.log('data=' + data)
            socket.emit("get uptime", data.toString() )
        })
    })
})


//buffers for recv and send to c app

//nodeJS to C app
udpSocket.on('error', (err) => {
    console.log('socket error:' + err.stack)
    udpSocket.close()
})

udpSocket.on("message", (msg, rinfo) => {                                           //Receive a Msg from C app
    console.log("server got " + msg + " from " + rinfo.address+ ":" + rinfo.port)
    var res = msg.toString()
    var resCheck = res.substr(0,4)
    if (resCheck == "vol:") {
        io.sockets.emit("set volume", res.substr(4) )
    }
    else if (resCheck == "bpm:") {
        io.sockets.emit("set bpm", res.substr(4) )
    }
    else if (resCheck=="def=") {
        io.sockets.emit("get default", res.substr(4))
    }
    else if (resCheck=="DRUM") {
        io.sockets.emit("set drum")
    }
    else if (resCheck=="mode") {
        io.sockets.emit("set mode", res.substr(5))
    }
})

udpSocket.on("listening", ()=> {
    const address = udpSocket.address()
    console.log('server listening :'+ address.address+" : " + address.port )
})

udpSocket.bind(udpPort)


// fs.readFile('/proc/uptime', function(err, data) {
//     console.log('data=' + data)
// })

