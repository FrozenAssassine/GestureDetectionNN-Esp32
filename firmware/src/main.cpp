#include "nn/layers.h"
#include "nn/neuralNetwork.h"
#include <nn/predictionHelper.h>
#include <Arduino.h>
#include "nn_trained.h"
#include "collectAccelData.h"

bool recordingMode = false; // set to true for recording data

NeuralNetwork *nn = new NeuralNetwork(3);

void initModel()
{
  nn->StackLayer(new InputLayer(13));
  nn->StackLayer(new DenseLayer(32, ActivationKind::Relu));
  nn->StackLayer(new OutputLayer(6, ActivationKind::Softmax));

  nn->LoadTrainedData(nn_layers, nn_total_layers);

  nn->Build(true);
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  initCollection();
  initModel();
}

void loop()
{
  if (recordingMode)
  {
    startCollection();
    return;
  }

  if (Serial.available())
  {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    Serial.println("Enter pressed");
    delay(1000);

    if (cmd.length() == 0)
    {
      Serial.println("Started recording");
      float data[13];
      if (collectFeatures(data))
      {
        Serial.println("Predictions:");

        float *pred = nn->Predict(data);
        Serial.printf(
            "Input: [%.0f, %.0f] -> Softmax: [%.4f, %.4f, %.4f, %.4f, %.4f, %.4f] -> Class: %d\n",
            data[0], data[1], pred[0], pred[1], pred[2], pred[3], pred[4], pred[5], (ArgMax(pred, 6) + 1));
      }
      else
      {
        Serial.println("Feature collection failed");
      }
    }
  }
  delay(1000);
}