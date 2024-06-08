var admin = require("firebase-admin");
const express = require('express');
var serviceAccount = require("./serviceAccountKey.json");

const app = express();

admin.initializeApp({
  credential: admin.credential.cert(serviceAccount),
  databaseURL: "https://buthtub-firebase-default-rtdb.firebaseio.com/" //thay thế URL DB
});

// Lấy reference đến Realtime Database
const db = admin.database();
const ref = db.ref('PTIOT_DHT');

// Route để hiển thị dữ liệu trên trang web
app.get('/', (req, res) => {
    // Đọc dữ liệu từ Realtime Database
    ref.once('value', snapshot => {
        const data = snapshot.val();
        const temperature = data.Temp;
        const humidity = data.hum;

        // Render trang web với dữ liệu từ Firebase
        res.send(`
            <h1>Hiển thị dữ liệu từ Firebase Realtime Database</h1>
            <h2>Minh họa kết nối Wokwi (Virtual IoT network) -> Firebase (RTDB) -> NodeJS (Webapp)</h2>
            <h3>* Nhiệt độ: ${temperature}°C</h3>
            <h3>* Độ ẩm: ${humidity}%</h3>
        `);
    }).catch(error => {
        console.error('Lỗi khi đọc dữ liệu:', error);
        res.status(500).send('Đã xảy ra lỗi khi đọc dữ liệu từ Firebase.');
    });
});

// Khởi động server
const port = 3000;
app.listen(port, () => {
    console.log(`Server đang lắng nghe tại http://localhost:${port}`);
});