class XYZ {
    public double X;
    public double Y;
    public double Z;
}

class Quaternion {
    public double X;
    public double Y;
    public double Z;
    public double W;
}

class GPS {
    public double Longitude;
    public double Latitude;
    public int VisibleSatellites;
}

class ImuState {
    public XYZ Orientation;
    public XYZ Acceleration;
    public XYZ Gyroscope;
    public XYZ Magnetometer;
    public Quaternion Quaternion;
}

class IMU {
    public XYZ getOrientation();
    public XYZ getAcceleration();
    public XYZ getGyroscope();
    public XYZ getMagnetometer();
    public Quaternion getQuaternion();
}

class Barometer {
    private double zeroAltitude;
    public int64 getPressure();
    public double calculateAltitude() => bmp581.calcAltitude(getPressure()) - zeroAltitude;
    public double calculateSeaLevelAltitude(int64 pressure) => bmp581.calcAltitude(pressure);
    public void resetZeroAltitude() => zeroAltitude = calculateSeaLevelAltitude(getPressure());
    public int64 getTemperature();
}

class LoggerState {
    public bool WritingToLogsEnabled;
    public byte StoredLogsSessions;
}

class Logger {
    public void logData(char[] data);//max 32 bytes
    public bool writeLogsToSD();//return true if write to SD succeeded
    public void eraseLogs();
}

class RfState {
    public bool IsTxMode;
    public bool IsRxMode;
    public char[] RxAddress;
    public char[] TxAddress;
}

class RF {
    public void setTxAddress(char[] txAddress);//max 5 bytes
    public void setRxAddress(char[] rxAddress);//max 5 bytes
    public void setTxMode();
    public void setRxMode();
    public void sendData(char[] data);//max 32 bytes
    public bool hasReceivedData() => nrf24.isDataAvailable() == 1;
    public char[] getReceivedData();
}

class Servo {
    public void setServoAngle(byte servoNumber, int servoAngle);
}

class Lighter {
    public void FireLighter(byte lighterNumber);
}

class ColirOneState {
    public long Timestamp;
    public GPS GPS;
    public double Altitude;
    public int64 Temperature;
    public double VerticalVelocity;
    public ImuState ImuState;
    public RfState RfState;
    public LoggerState LoggerState;
}

class ColirOne {
    public IMU IMU;
    public Barometer Barometer;
    public Logger Logger;
    public RF RF;
    public Servo Servo;
    public Lighter Lighter;
    public ColirOneState State;

    private void setState() {
        //read all sensors data and set state
        //State.Timestamp = millis();
        //State.ImuState.Orientation = IMU.getOrientation();
        //etc.
    }

    public void init();
    public void base() {
        setState();
    }
}
