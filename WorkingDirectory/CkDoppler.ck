//fun void UCHUCK() {};

// Define global variables for velocities
global float sourceVelocity;
global float observerVelocity;

// Initialize velocities
50.0 => sourceVelocity;    // Velocity of the source in meters per second
0.0 => observerVelocity;   // Velocity of the observer in meters per second

// Define the speed of sound in meters per second
343.0 => float speedOfSound;

// Create an ADC (audio input) and connect it to a PitShift
adc => PitShift p => dac;
p.mix(1.0); // Fully apply the pitch shift

// Function to calculate the pitch shift factor using the Doppler effect formula
fun float calculatePitchShift(float sourceVel, float observerVel) {
    return (speedOfSound + observerVel) / (speedOfSound - sourceVel);
}

// Continuously update the pitch shift based on the current velocities
while (true) {
    // Calculate the pitch shift factor
    calculatePitchShift(sourceVelocity, observerVelocity) => float pitchShift;
    
    // Apply the pitch shift factor to the PitShift
    pitchShift => p.shift;
    
   samp => now; // Update every 10 milliseconds
}