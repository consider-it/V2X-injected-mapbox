package com.mapbox.rctmgl.models;

public class TlSpat {
    private final int direction;
    private final int phase;
    private final int remainingSec;
    private final int advisedSpeedKph;

    public TlSpat(int d, int p, int r, int a) {
        this.direction = d;
        this.phase = p;
        this.remainingSec = r;
        this.advisedSpeedKph = a;
    }

    public String toJson() {

        return "{\"direction\":" +
                this.direction +
                ",\"phase\":" +
                this.phase +
                ",\"remainingSec\":" +
                this.remainingSec +
                ",\"advisedSpeedKph\":" +
                this.advisedSpeedKph +
                "}";
    }

    public int getDirection() {
        return this.direction;
    }

    public int getPhase() {
        return this.phase;
    }

    public int getRemainingSec() {
        return this.remainingSec;
    }

    public int getAdvisedSpeedKph() {
        return this.advisedSpeedKph;
    }

    @Override
    public String toString() {
        return "TlSpat {" + this.direction + ", " + this.phase + ", " + this.remainingSec + ", " + this.advisedSpeedKph
                + "}";
    }
}