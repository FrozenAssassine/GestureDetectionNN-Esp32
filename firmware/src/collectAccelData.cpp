#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// mpu 6050 pins
#define SDA_PIN 21
#define SCL_PIN 22

#define MAX_RECORDS 100

struct FeatureRow
{
    int label;
    float features[13];
};

FeatureRow records[MAX_RECORDS];
int recordCount = 0;
int currentLabel = -1;

const unsigned long RECORD_MS = 3000;
const unsigned long SAMPLE_INTERVAL_MS = 10;

Adafruit_MPU6050 mpu;

struct AxisStats
{
    float sum = 0;
    float sumSq = 0;
    float minVal = 1e9;
    float maxVal = -1e9;
    int count = 0;

    void add(float v)
    {
        sum += v;
        sumSq += v * v;
        if (v < minVal)
            minVal = v;
        if (v > maxVal)
            maxVal = v;
        count++;
    }

    float mean() const
    {
        // the average value
        return sum / count;
    }

    float stddev() const
    {
        // how far away are values of the average
        float m = mean();
        return sqrt((sumSq / count) - (m * m));
    }

    float rms() const
    {
        // an average which highlights larger values
        return sqrt(sumSq / count);
    }

    float p2p() const
    {
        // difference between the min and max val.
        return maxVal - minVal;
    }
};

void initCollection()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(10);
    }
    Wire.begin(SDA_PIN, SCL_PIN);

    if (!mpu.begin())
    {
        Serial.println("Failed to find MPU6050 chip. Check wiring.");
        while (1)
        {
            delay(1000);
        }
    }

    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    Serial.println();
    Serial.println("Press ENTER to record a movement...");
}

bool recordOnce()
{
    if (recordCount >= MAX_RECORDS)
    {
        Serial.println("Record buffer full!");
        return false;
    }

    AxisStats ax, ay, az;
    float amagSumSq = 0;
    int amagCount = 0;

    unsigned long start = millis();
    unsigned long nextSample = millis() + SAMPLE_INTERVAL_MS;

    while (millis() - start < RECORD_MS)
    {
        if (millis() >= nextSample)
        {
            sensors_event_t a, g, temp;
            mpu.getEvent(&a, &g, &temp);

            ax.add(a.acceleration.x);
            ay.add(a.acceleration.y);
            az.add(a.acceleration.z);

            float amag = sqrt(
                a.acceleration.x * a.acceleration.x +
                a.acceleration.y * a.acceleration.y +
                a.acceleration.z * a.acceleration.z);

            amagSumSq += amag * amag;
            amagCount++;

            nextSample += SAMPLE_INTERVAL_MS;
        }
        delay(1);
    }

    FeatureRow &r = records[recordCount++];
    r.label = currentLabel;

    r.features[0] = ax.mean();
    r.features[1] = ax.stddev();
    r.features[2] = ax.rms();
    r.features[3] = ax.p2p();

    r.features[4] = ay.mean();
    r.features[5] = ay.stddev();
    r.features[6] = ay.rms();
    r.features[7] = ay.p2p();

    r.features[8] = az.mean();
    r.features[9] = az.stddev();
    r.features[10] = az.rms();
    r.features[11] = az.p2p();

    r.features[12] = sqrt(amagSumSq / amagCount);

    Serial.print("Recorded sample ");
    Serial.println(recordCount);

    return true;
}

bool collectFeatures(float out[13])
{
    AxisStats ax, ay, az;
    float amagSumSq = 0;
    int amagCount = 0;

    unsigned long start = millis();
    unsigned long nextSample = start;

    while (millis() - start < RECORD_MS)
    {
        if (millis() >= nextSample)
        {
            sensors_event_t a, g, temp;
            mpu.getEvent(&a, &g, &temp);

            ax.add(a.acceleration.x);
            ay.add(a.acceleration.y);
            az.add(a.acceleration.z);

            float amag = sqrt(
                a.acceleration.x * a.acceleration.x +
                a.acceleration.y * a.acceleration.y +
                a.acceleration.z * a.acceleration.z);

            amagSumSq += amag * amag;
            amagCount++;

            nextSample += SAMPLE_INTERVAL_MS;
        }
        delay(1);
    }

    if (amagCount == 0)
        return false;

    out[0] = ax.mean();
    out[1] = ax.stddev();
    out[2] = ax.rms();
    out[3] = ax.p2p();

    out[4] = ay.mean();
    out[5] = ay.stddev();
    out[6] = ay.rms();
    out[7] = ay.p2p();

    out[8] = az.mean();
    out[9] = az.stddev();
    out[10] = az.rms();
    out[11] = az.p2p();

    out[12] = sqrt(amagSumSq / amagCount);

    return true;
}

void startCollection()
{
    if (currentLabel < 0)
    {
        Serial.println("Enter movement label (integer):");
        while (!Serial.available())
            delay(10);
        currentLabel = Serial.readStringUntil('\n').toInt();

        Serial.print("Using label: ");
        Serial.println(currentLabel);
        Serial.println("Press ENTER to record.");
        Serial.println("'p' + ENTER → print CSV");
        Serial.println("'c' + ENTER → clear buffer");
        return;
    }

    if (Serial.available())
    {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();

        if (cmd.length() == 0)
        {
            Serial.println("Recording...");
            recordOnce();
        }
        else if (cmd == "p")
        {
            Serial.println("label,ax_mean,ax_std,ax_rms,ax_p2p,"
                           "ay_mean,ay_std,ay_rms,ay_p2p,"
                           "az_mean,az_std,az_rms,az_p2p,amag_rms");

            for (int i = 0; i < recordCount; i++)
            {
                Serial.print(records[i].label);
                for (int j = 0; j < 13; j++)
                {
                    Serial.print(',');
                    Serial.print(records[i].features[j], 6);
                }
                Serial.println();
            }

            Serial.println("---END---");
        }
        else if (cmd == "c")
        {
            recordCount = 0;
            Serial.println("Buffer cleared.");
        }
    }
}
