package com.mapbox.rctmgl.models;

public class ObuInfo {
    private final double lat;
    private final double lon;
    private final double heading;
    private final double kphSpeed;
    private final double time;

    public ObuInfo(double lon, double lat, double heading, double speed, double time) {
        this.lat = lat;
        this.lon = lon;
        this.heading = heading;
        this.kphSpeed = speed;
        this.time = time;
    }

    public String toJson() {

        return "{\"type\":\"Feature\",\"geometry\":{\"type\":\"Point\",\"coordinates\":[" +
                this.lon +
                "," +
                this.lat +
                "]},\"properties\":{\"heading\":" +
                this.heading +
                ",\"kphSpeed\":" +
                this.kphSpeed +
                "}}";
    }

    public double getLat() {
        return this.lat;
    }

    public double getLon() {
        return this.lon;
    }

    public double getHeading() {
        return this.heading;
    }

    public double getKphSpeed() {
        return this.kphSpeed;
    }

    public double getTime() {
        return this.time;
    }
}