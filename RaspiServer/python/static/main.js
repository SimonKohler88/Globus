// Interval ID for motor speed updates
let speedUpdateInterval = null;

// Initialize on page load
document.addEventListener('DOMContentLoaded', function () {
    startSpeedUpdates();
});

// Send device time to server when page loads
window.addEventListener('DOMContentLoaded', function () {
    const currentTime = new Date().toString();

    fetch('/device_time', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({time: currentTime})
    })
        .then(response => response.json())
        .then(data => {
            console.log('Time sent successfully:', data);
        })
        .catch(error => {
            console.error('Error sending time:', error);
        });
});

// Start periodic speed updates
function startSpeedUpdates(intervalMs = 500) {
    // Clear any existing interval
    if (speedUpdateInterval) {
        clearInterval(speedUpdateInterval);
    }

    // Fetch speed immediately
    updateMotorSpeed();

    // Set up periodic updates
    speedUpdateInterval = setInterval(updateMotorSpeed, intervalMs);
}

// Stop periodic speed updates
function stopSpeedUpdates() {
    if (speedUpdateInterval) {
        clearInterval(speedUpdateInterval);
        speedUpdateInterval = null;
    }
}

// Fetch and update motor speed from server
async function updateMotorSpeed() {
    try {
        const response = await fetch('/motor_speed');
        if (!response.ok) {
            console.error('Failed to fetch motor speed');
            return;
        }

        const data = await response.json();

        // Update current speed display
        const currSpeedElement = document.getElementById('curr_speed');
        if (currSpeedElement) {
            currSpeedElement.textContent = data.speed;
        }

        // Update speed meter
        const speedMeterElement = document.getElementById('speed_meter');
        if (speedMeterElement) {
            speedMeterElement.value = data.speed;
        }

        // Update slider position
        const sliderElement = document.getElementById('mySlider');
        if (sliderElement) {
            sliderElement.value = data.target_speed;
        }

        // Update slider value background display
        const sliderValueBgElement = document.getElementById('sliderValueBg');
        if (sliderValueBgElement) {
            sliderValueBgElement.textContent = data.target_speed;
        }

        // Update button states based on enabled status
        updateButtonStates(data.enabled);

    } catch (error) {
        console.error('Error updating motor speed:', error);
    }
}

// Update button enable/disable states based on motor enabled status
function updateButtonStates(enabled) {
    const eStopButton = document.getElementById('eStopButton');
    const resetButton = document.getElementById('resetButton');

    if (enabled) {
        // Motor is enabled: E-Stop active, Reset inactive
        if (eStopButton) {
            eStopButton.classList.remove('btn-disabled');
            eStopButton.classList.add('btn-enabled');
            eStopButton.disabled = false;
        }
        if (resetButton) {
            resetButton.classList.remove('btn-enabled');
            resetButton.classList.add('btn-disabled');
            resetButton.disabled = true;
        }
    } else {
        // Motor is disabled: E-Stop inactive, Reset active
        if (eStopButton) {
            eStopButton.classList.remove('btn-enabled');
            eStopButton.classList.add('btn-disabled');
            eStopButton.disabled = true;
        }
        if (resetButton) {
            resetButton.classList.remove('btn-disabled');
            resetButton.classList.add('btn-enabled');
            resetButton.disabled = false;
        }
    }
}

// Submit motor control command
function submit_motor_control(cmd) {
    fetch(`/motor_control/${cmd}`)
        .then(response => {
            if (!response.ok) {
                console.error('Motor control command failed');
            }
        })
        .catch(error => console.error('Error:', error));
    return false;
}

// Submit JPEG upload form
function submit_jpeg() {
    const form = document.getElementById('id_jpeg_upload_form');
    const formData = new FormData(form);

    fetch('/upload', {
        method: 'POST',
        body: formData
    })
        .then(response => {
            if (!response.ok) {
                console.error('Upload failed');
            }
        })
        .catch(error => console.error('Error:', error));

    return false;
}

// Submit globe view selection
function submit_globe_view() {
    const form = document.getElementById('id_globe_selector');
    const formData = new FormData(form);

    fetch('/show_globe_video', {
        method: 'POST',
        body: formData
    })
        .then(response => {
            if (!response.ok) {
                console.error('Globe view submission failed');
            }
        })
        .catch(error => console.error('Error:', error));

    return false;
}

