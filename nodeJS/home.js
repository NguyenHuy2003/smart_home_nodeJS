const admin = require("firebase-admin");
const express = require('express');
const http = require('http');
const socketIo = require('socket.io');
const serviceAccount = require("./serviceAccountKey.json");

const app = express();
const server = http.createServer(app);
const io = socketIo(server);

admin.initializeApp({
  credential: admin.credential.cert(serviceAccount),
  databaseURL: "https://flutter-firebase-app-90c9a-default-rtdb.firebaseio.com/"
});

const db = admin.database();
const ref = db.ref('PTIOT_DHT');

app.get('/', (req, res) => {
    res.send(`
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Smart Home</title>
        <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
        <script src="https://cdn.jsdelivr.net/npm/chartjs-adapter-date-fns"></script>
        <script src="https://cdn.socket.io/4.0.0/socket.io.min.js"></script>
        <style>
            body {
                font-family: Arial, sans-serif;
                margin: 0;
                padding: 0;
                background-color: #f4f4f9;
                color: #333;
            }
            h1, h2 {
                text-align: center;
                margin: 20px 0;
            }
            .container {
                max-width: 900px;
                margin: 0 auto;
                padding: 20px;
                background-color: #fff;
                box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
                border-radius: 8px;
            }
            .info {
                display: flex;
                flex-direction: column;
                margin-bottom: 20px;
            }
            .info p {
                margin: 5px 0;
                font-size: 1.2em;
            }
            button {
                padding: 10px 20px;
                font-size: 1em;
                background-color: #007bff;
                color: #fff;
                border: none;
                border-radius: 5px;
                cursor: pointer;
                margin-top: 10px;
            }
            button:hover {
                background-color: #0056b3;
            }
            table {
                width: 100%;
                border-collapse: collapse;
                margin-top: 20px;
                word-wrap: break-word;
                table-layout: auto;
            }
            th, td {
                padding: 12px;
                text-align: left;
                border-bottom: 1px solid #ddd;
            }
            th {
                background-color: #007bff;
                color: white;
            }
            canvas {
                max-width: 100%;
                height: auto;
            }
        </style>
    </head>
    <body>
        <div class="container">
            <h1>Thông tin thời gian thực</h1>
            <div class="info">
                <p id="dis">Cảm biến khoảng cách: </p>
                <p id="led-01">Led 01: </p>
                <p id="led-02">Led 02: </p>
                <p id="led-03">Led 03: </p>
                <p id="led-04">Led 04 - Cảm biến vật thể: </p>
                <button id="toggle-led-01">Bật/Tắt Led 01</button>
            </div>
            <h2>Biểu đồ nhiệt độ và độ ẩm theo thời gian thực</h2>
            <canvas id="chart" width="400" height="200"></canvas>
            <h2>Dữ liệu thay đổi theo thời gian thực</h2>
            <table id="data-table">
                <thead>
                    <tr>
                        <th>Thời gian</th>
                        <th>Nhiệt độ (°C)</th>
                        <th>Độ ẩm (%)</th>
                        <th>Cảm biến khoảng cách</th>
                        <th>Led 01</th>
                        <th>Led 02</th>
                        <th>Led 03</th>
                        <th>Led 04 - Cảm biến vật thể</th>
                    </tr>
                </thead>
                <tbody></tbody>
            </table>
        </div>

        <script>
            const ctx = document.getElementById('chart').getContext('2d');
            const chart = new Chart(ctx, {
                type: 'line',
                data: {
                    labels: [],
                    datasets: [
                        {
                            label: 'Nhiệt độ (°C)',
                            data: [],
                            borderColor: 'red',
                            fill: false
                        },
                        {
                            label: 'Độ ẩm (%)',
                            data: [],
                            borderColor: 'blue',
                            fill: false
                        }
                    ]
                },
                options: {
                    scales: {
                        x: {
                            type: 'time',
                            time: {
                                unit: 'minute',
                                tooltipFormat: 'yyyy-MM-dd HH:mm:ss'
                            }
                        },
                        y: {
                            beginAtZero: true
                        }
                    }
                }
            });

            const socket = io();

            socket.on('data', (data) => {
                const { temperature, humidity, dis, led_01, led_02, led_03, led_04, timestamp } = data;

                console.log('Data received:', data);

                // Update displayed info
                document.getElementById('dis').innerText = 'Cảm biến khoảng cách: ' + dis;
                document.getElementById('led-01').innerText = 'Led 01: ' + (led_01 === 1 ? 'Bật' : 'Tắt');
                document.getElementById('led-02').innerText = 'Led 02: ' + (led_02 === 1 ? 'Bật' : 'Tắt');
                document.getElementById('led-03').innerText = 'Led 03: ' + (led_03 === 1 ? 'Bật' : 'Tắt');
                document.getElementById('led-04').innerText = 'Led 04 - Cảm biến vật thể: ' + (led_04 === 14 ? 'Tắt' : 'Bật');

                if (chart.data.labels.length > 50) {
                    chart.data.labels.shift();
                    chart.data.datasets[0].data.shift();
                    chart.data.datasets[1].data.shift();    
                }

                chart.data.labels.push(timestamp);
                chart.data.datasets[0].data.push(temperature);
                chart.data.datasets[1].data.push(humidity);

                chart.update();

                // Update data table
                const dataTable = document.getElementById('data-table').getElementsByTagName('tbody')[0];
                const newRow = dataTable.insertRow();
                newRow.innerHTML = \`
                    <td>\${timestamp}</td>
                    <td>\${temperature}</td>
                    <td>\${humidity}</td>
                    <td>\${dis}</td>
                    <td>\${led_01 === 1 ? 'Bật' : 'Tắt'}</td>
                    <td>\${led_02 === 1 ? 'Bật' : 'Tắt'}</td>
                    <td>\${led_03 === 1 ? 'Bật' : 'Tắt'}</td>
                    <td>\${led_04 === 14 ? 'Tắt' : 'Bật'}</td>
                \`;
            });

            document.getElementById('toggle-led-01').addEventListener('click', () => {
                socket.emit('toggle-led-01');
            });

            socket.on('connect', () => {
                console.log('Connected to server');
            });

            socket.on('disconnect', () => {
                console.log('Disconnected from server');
            });

            socket.on('error', (err) => {
                console.error('Socket error:', err);
            });
        </script>
    </body>
    </html>
    `);
});


ref.on('value', snapshot => {
    const data = snapshot.val();
    const temperature = data.Temp;
    const humidity = data.hum;
    const dis = data.Dis;
    const led_01 = data.LED_01;
    const led_02 = data.LED_02;
    const led_03 = data.LED_03;
    const led_04 = data.LED_04;
    const timestamp = new Date().toISOString();

    console.log('Data from Firebase:', { temperature, humidity, dis, led_01, led_02, led_03, led_04, timestamp });

    io.emit('data', { temperature, humidity, dis, led_01, led_02, led_03, led_04, timestamp });
});

io.on('connection', (socket) => {
    socket.on('toggle-led-01', () => {
        ref.once('value', snapshot => {
            const currentLedState = snapshot.val().LED_01;
            const newLedState = currentLedState === 1 ? 0 : 1;
            ref.update({ LED_01: newLedState });
        });
    });

    socket.on('disconnect', () => {
        console.log('Client disconnected');
    });
});

const port = 2003;
server.listen(port, () => {
    console.log(`Server đang lắng nghe tại http://localhost:${port}`);
});
