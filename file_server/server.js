const express = require('express');
const cors = require('cors');
const app = express();
const port = process.env.PORT || 5000;
// require('dotenv/config');

app.use(express.json());
app.use(cors);

// const authRoute = require('./routes/Auth/auth');
const gameRoute = require('./routes/Game/index');

// app.use('/auth', authRoute);
app.use('/game', gameRoute);

app.get('/', (req, res) => {
    console.log("1 req came!");
    res.send('This is home page');
})


app.listen(port, () => {
    console.log(`Listening on port ${port}...`);
});