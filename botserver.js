const express = require('express');
const { spawn } = require('child_process');
const app = express();
const port = 3000;

app.use(express.json());

let botProcess;
let botBuffer = '';

const cors = require('cors');
app.use(cors());


function startBotProcess() {
    botProcess = spawn('./bin/Gnizabalone.exe');

    botProcess.stdout.on('data', (data) => {
        botBuffer += data.toString();
    });

    botProcess.stderr.on('data', (data) => {
        console.error(`Bot stderr: ${data}`);
    });

    botProcess.on('close', (code) => {
        console.log(`Bot exited with code ${code}`);
        botProcess = null;
    });
}

function sendCommandToBot(command) {
    return new Promise((resolve) => {
        let outputBuffer = '';

        botProcess.stdin.write(command + '\n');

        const onData = (data) => {
            outputBuffer += data.toString();

            const lines = outputBuffer.split('\n').map(line => line.trim());

            if (lines.some(line => line.toLowerCase() === 'invalid move')) {
                cleanup();
                resolve('Invalid move');
                return;
            }

            const boardLine = lines.find(line => line.startsWith('boardString:'));
            const evalLine = lines.find(line => line.startsWith('eval:'));

            if (boardLine) {
                cleanup();
                const result = {
                    boardString: boardLine.slice('boardString:'.length).trim()
                };
                if (evalLine) {
                    result.eval = evalLine.slice('eval:'.length).trim();
                }
                resolve(result);
                return;
            }
        };

        const cleanup = () => {
            botProcess.stdout.off('data', onData);
            clearTimeout(timeout);
        };

        botProcess.stdout.on('data', onData);

        const timeout = setTimeout(() => {
            cleanup();
            resolve(outputBuffer.trim() || 'Timeout: No valid response from bot');
        }, 40000); // 40 seconds max
    });
}

// Route: Start Bot
app.post('/start-bot', async (req, res) => {
    console.log('Received /start-bot request:', req.body);
    if (!botProcess) {
        startBotProcess();
    }

    const { command } = req.body;
    console.log('Command to send:', command);

    if (command) {
        const response = await sendCommandToBot(command);
        res.json({ boardString: response });
        console.log('Response sent:', response);
    } else {
        res.json({ error: 'No command provided' });
    }
});

// Route: Send Move
app.post('/send-move', async (req, res) => {
    console.log('Received /send-move request:', req.body);

    if (!botProcess) {
        return res.json({ error: 'Bot not running' });
    }

    const response = await sendCommandToBot(req.body.command);
    console.log('Response from bot:', response); 
    res.json({ response });
});

app.post('/send-go', async (req, res) => {
    console.log('Received /send-go request:', req.body);
    if (!botProcess) {
        return res.json({ error: 'Bot not running' });
    }
    response = await sendCommandToBot(req.body.command);
    console.log('Response from bot:', response);
    if (typeof response === 'string' && response.includes('Game not started')) {
        return res.json({ error: 'Game not started error' });
    }
    else if (typeof response === 'string' && response.includes('best so far')) {
        // we need to kill the bot process and start a new one
        botProcess.kill();
        startBotProcess();
        return res.json({ error: 'not finished error' });
    }
    res.json({ response });
});

app.listen(port, () => {
    console.log(`Server running on http://localhost:${port}`);
});

