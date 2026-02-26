// State management
const appState = {
    targetTemp: 30,
    timerSeconds: 0,
    timerActive: false,
    timerMode: 'OFF',
    timerDurationSeconds: 0,
    timerOriginalSeconds: 0,
    preHeatMinutes: 15,
    selectedPower: '5V',
    ch1Temp: 25.4,
    ch2Temp: 25.4
};

// Timer interval reference
let timerInterval = null;

// Initialize app on page load
document.addEventListener('DOMContentLoaded', () => {
    initializeEventListeners();
    updateDisplay();
});

// Event listeners
function initializeEventListeners() {
    // Temperature control buttons
    document.getElementById('decreaseBtn').addEventListener('click', decreaseTemp);
    document.getElementById('increaseBtn').addEventListener('click', increaseTemp);
    
    // Timer control
    document.getElementById('startBtn').addEventListener('click', toggleTimer);
    
    // Timer Mode toggle
    document.getElementById('timerModeBtn').addEventListener('click', toggleTimerMode);
    
    // Pre-Heat control
    document.getElementById('preHeatBtn').addEventListener('click', openPreHeatPicker);
    
    // Pre-Heat modal buttons
    document.getElementById('preHeatConfirm').addEventListener('click', confirmPreHeat);
    document.getElementById('preHeatCancel').addEventListener('click', closePreHeatModal);
    
    // Timer Mode modal buttons
    document.getElementById('timerModeConfirm').addEventListener('click', confirmTimerMode);
    document.getElementById('timerModeCancel').addEventListener('click', closeTimerModeModal);
    
    // Timer Mode modal sliders
    document.getElementById('hoursSlider').addEventListener('input', updateHoursDisplay);
    document.getElementById('minutesSlider').addEventListener('input', updateMinutesDisplay);
    
    // Power selection buttons are read-only (controlled by ESP32)
    // No click handlers - these are display only
    
    // WiFi button
    document.getElementById('wifiBtn').addEventListener('click', handleWiFiClick);
}

// Temperature control functions
function decreaseTemp() {
    if (appState.targetTemp > 0) {
        appState.targetTemp--;
        updateTargetTempDisplay();
        sendStateUpdate('targetTemp', appState.targetTemp);
    }
}

function increaseTemp() {
    if (appState.targetTemp < 100) {
        appState.targetTemp++;
        updateTargetTempDisplay();
        sendStateUpdate('targetTemp', appState.targetTemp);
    }
}

function updateTargetTempDisplay() {
    document.getElementById('targetTemp').textContent = appState.targetTemp;
}

// Timer functions
function toggleTimer() {
    appState.timerActive = !appState.timerActive;
    const startText = document.querySelector('.start-text');
    const timerModeBtn = document.getElementById('timerModeBtn');
    const preHeatBtn = document.getElementById('preHeatBtn');
    
    if (appState.timerActive) {
        startText.textContent = 'STOP';
        timerModeBtn.disabled = true; // Disable TIMER MODE when running
        preHeatBtn.disabled = true; // Disable PRE-HEAT when running
        
        if (appState.timerMode === 'ON') {
            // Countdown timer - reset to original duration
            appState.timerSeconds = appState.timerOriginalSeconds;
            startCountdownTimer();
        } else {
            // Count up timer
            startTimer();
        }
        sendStateUpdate('timerActive', true);
    } else {
        startText.textContent = 'START';
        timerModeBtn.disabled = false; // Enable TIMER MODE when stopped
        preHeatBtn.disabled = false; // Enable PRE-HEAT when stopped
        appState.timerSeconds = 0;
        stopTimer();
        updateTimerDisplay();
        sendStateUpdate('timerActive', false);
    }
}

function startTimer() {
    if (timerInterval) clearInterval(timerInterval);
    
    updateTimerDisplay();
    addBlinkingColon();
    
    timerInterval = setInterval(() => {
        appState.timerSeconds++;
        updateTimerDisplay();
    }, 1000);
}

function startCountdownTimer() {
    if (timerInterval) clearInterval(timerInterval);
    
    // Display current time immediately before starting countdown
    updateTimerDisplay();
    addBlinkingColon();
    
    timerInterval = setInterval(() => {
        appState.timerSeconds--;
        updateTimerDisplay();
        
        if (appState.timerSeconds <= 0) {
            // Timer expired
            removeBlinkingColon();
            stopTimer();
            toggleTimer(); // Stop the timer
        }
    }, 1000);
}

function addBlinkingColon() {
    const timerDisplay = document.getElementById('timerDisplay');
    // Create a span for the colon if it doesn't exist
    if (!timerDisplay.querySelector('.blink-colon')) {
        const text = timerDisplay.textContent;
        const colonIndex = text.indexOf(':');
        if (colonIndex !== -1) {
            const beforeColon = text.substring(0, colonIndex);
            const afterColon = text.substring(colonIndex + 1);
            timerDisplay.innerHTML = beforeColon + '<span class="blink-colon">:</span>' + afterColon;
        }
    }
}

function removeBlinkingColon() {
    const timerDisplay = document.getElementById('timerDisplay');
    const blinkSpan = timerDisplay.querySelector('.blink-colon');
    if (blinkSpan) {
        const text = timerDisplay.textContent;
        timerDisplay.textContent = text;
    }
}

function stopTimer() {
    if (timerInterval) {
        clearInterval(timerInterval);
        timerInterval = null;
    }
    removeBlinkingColon();
}

function updateTimerDisplay() {
    const timerDisplay = document.getElementById('timerDisplay');
    let timeStr;
    
    if (appState.timerMode === 'ON') {
        // Display hours and minutes
        const hours = Math.floor(appState.timerSeconds / 3600);
        const minutes = Math.floor((appState.timerSeconds % 3600) / 60);
        timeStr = `${String(hours).padStart(2, '0')}:${String(minutes).padStart(2, '0')}`;
    } else {
        // Display minutes and seconds
        const minutes = Math.floor(appState.timerSeconds / 60);
        const seconds = appState.timerSeconds % 60;
        timeStr = `${String(minutes).padStart(2, '0')}:${String(seconds).padStart(2, '0')}`;
    }
    
    // Check if colon should be blinking (timer is active)
    if (appState.timerActive && timerDisplay.querySelector('.blink-colon')) {
        // Preserve the blinking colon span
        const colonIndex = timeStr.indexOf(':');
        if (colonIndex !== -1) {
            const beforeColon = timeStr.substring(0, colonIndex);
            const afterColon = timeStr.substring(colonIndex + 1);
            timerDisplay.innerHTML = beforeColon + '<span class="blink-colon">:</span>' + afterColon;
        }
    } else {
        // Normal text display (no blinking)
        timerDisplay.textContent = timeStr;
    }
}

// Power selection - READ ONLY (controlled by ESP32)
// This function is kept for internal state updates only
function selectPower(voltage) {
    appState.selectedPower = voltage;
    
    // Update button states
    document.querySelectorAll('.power-btn').forEach(btn => {
        btn.classList.remove('active');
    });
    
    // Find and highlight the button with matching voltage
    document.querySelectorAll('.power-btn').forEach(btn => {
        if (btn.textContent === voltage) {
            btn.classList.add('active');
        }
    });
}

// Status display update
function updateStatusDisplay() {
    document.getElementById('timerMode').textContent = appState.timerMode;
    document.getElementById('preHeat').textContent = appState.preHeatMinutes + 'm';
}

// General display update
function updateDisplay() {
    updateTargetTempDisplay();
    updateTimerDisplay();
    updateStatusDisplay();
}

// WiFi button handler
function handleWiFiClick() {
    console.log('WiFi button clicked');
    alert('WiFi connection feature coming soon!');
    sendStateUpdate('wifiRequest', true);
}

// Timer Mode toggle
function toggleTimerMode() {
    if (appState.timerMode === 'OFF') {
        // Turning ON - show timer picker
        openTimerModeModal();
    } else {
        // Turning OFF - just toggle
        appState.timerMode = 'OFF';
        document.getElementById('timerMode').textContent = appState.timerMode;
        sendStateUpdate('timerMode', appState.timerMode);
    }
}

// Pre-Heat picker
function openPreHeatPicker() {
    const modal = document.getElementById('preHeatModal');
    const slider = document.getElementById('preHeatSlider');
    const sliderValue = document.getElementById('sliderValue');
    
    slider.value = appState.preHeatMinutes;
    sliderValue.textContent = appState.preHeatMinutes + 'm';
    
    modal.classList.remove('hidden');
    modal.classList.add('visible');
    
    // Update value display as slider moves
    slider.addEventListener('input', () => {
        sliderValue.textContent = slider.value + 'm';
    });
}

function closePreHeatModal() {
    const modal = document.getElementById('preHeatModal');
    modal.classList.remove('visible');
    modal.classList.add('hidden');
}

function confirmPreHeat() {
    const slider = document.getElementById('preHeatSlider');
    appState.preHeatMinutes = parseInt(slider.value);
    document.getElementById('preHeat').textContent = appState.preHeatMinutes + 'm';
    sendStateUpdate('preHeatMinutes', appState.preHeatMinutes);
    closePreHeatModal();
}

// Timer Mode modal
function openTimerModeModal() {
    const modal = document.getElementById('timerModeModal');
    const hoursSlider = document.getElementById('hoursSlider');
    const minutesSlider = document.getElementById('minutesSlider');
    
    // Calculate current hours and minutes from timerDurationSeconds
    const hours = Math.floor(appState.timerDurationSeconds / 3600);
    const minutes = Math.floor((appState.timerDurationSeconds % 3600) / 60);
    
    hoursSlider.value = hours;
    minutesSlider.value = minutes;
    
    updateHoursDisplay();
    updateMinutesDisplay();
    
    modal.classList.remove('hidden');
    modal.classList.add('visible');
}

function closeTimerModeModal() {
    const modal = document.getElementById('timerModeModal');
    modal.classList.remove('visible');
    modal.classList.add('hidden');
}

function updateHoursDisplay() {
    const hours = document.getElementById('hoursSlider').value;
    document.getElementById('hoursValue').textContent = String(hours).padStart(2, '0');
}

function updateMinutesDisplay() {
    const minutes = document.getElementById('minutesSlider').value;
    document.getElementById('minutesValue').textContent = String(minutes).padStart(2, '0');
}

function confirmTimerMode() {
    const hours = parseInt(document.getElementById('hoursSlider').value);
    const minutes = parseInt(document.getElementById('minutesSlider').value);
    
    appState.timerDurationSeconds = hours * 3600 + minutes * 60;
    appState.timerOriginalSeconds = appState.timerDurationSeconds;
    appState.timerSeconds = appState.timerDurationSeconds;
    appState.timerMode = 'ON';
    
    document.getElementById('timerMode').textContent = appState.timerMode;
    updateTimerDisplay();
    sendStateUpdate('timerMode', appState.timerMode);
    
    closeTimerModeModal();
}

// API Communication (placeholder for future ESP32 integration)
function sendStateUpdate(key, value) {
    // This function will send updates to the ESP32 backend
    // For now, it just logs to console
    console.log(`State update: ${key} = ${value}`);
    
    // Future implementation will use fetch to send to ESP32:
    // fetch('/api/state', {
    //     method: 'POST',
    //     headers: { 'Content-Type': 'application/json' },
    //     body: JSON.stringify({ [key]: value })
    // });
}
