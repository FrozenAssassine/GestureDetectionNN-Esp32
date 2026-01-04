# Local Gesture Detection NN on Esp32

This project lets you record gestures with an ESP32, train them in PyTorch, and run them back on device using a custom neural network I wrote. It’s designed to be flexible, so you can easily train and swap your own gestures without ever having to hardcode a single movement.

## ⚙️ How it works

The workflow is split into three steps:

1. **Record**: Use the ESP32 and an accelerometer to capture movements. Data is sent over Serial and manually saved to a CSV.
2. **Train**: Run the Python scripts to process your data and train a PyTorch model.
3. **Deploy**: The script exports the model weights and biases as a C++ header file. Copy this back to your ESP32 to start recognizing gestures.

---

## 🧠 The Neural Network

The model is a simple feedforward network. I built the C++ implementation to mirror the PyTorch architecture exactly so the weights can be dropped right in.

- **Labels**: Currently set up for 4 custom gestures.
- **Performance**: Optimized to run fast on ESP32 hardware without heavy libraries.
- **Dependency**: This is based on my [Neural Network from scratch on ESP32/Arduino](https://github.com/FrozenAssassine/NeuralNetwork-Arduino).

## 🏗️ Quick Start

### 1. Data Collection

Flash the main.cpp and open your Serial monitor. Move the sensor, label your gestures, and save the output into your data file. The more data the better the accuracy. Aim for more than 20 recordings per gesture for best results! Press p in the serial console to print the csv and copy it into the data.csv file.

### 2. Training

Run the training script (main.py) in the Python folder. This will:

- Read your recordings.
- Train the model.

### 3. Model conversion

Run the script, that converts the torch model to the esp32 model header file.

- Run make_esp_model.py
- Copy the nn_trained into the include folder inside the firmware folder.

### 4. Inference

In the main.cpp, set the recordingMode to false and flash the code. Open the Serial monitor, press enter, perform a movement, and the ESP32 will tell you what gesture it detected.

## 📷 Prediction 
<img width="561" height="258" alt="image" src="https://github.com/user-attachments/assets/63069c87-d693-4a16-91d9-b99ede0ee0ef" />
