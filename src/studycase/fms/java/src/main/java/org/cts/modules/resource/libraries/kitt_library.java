package org.cts.modules.resource.libraries;

import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;

import java.io.IOException;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

import org.cts.utilities.Constants;

public class kitt_library {

    public static Semaphore kittBeingUsed = new Semaphore(1);

    public static void executeTransporte(String skillID) throws IOException {
        OkHttpClient client = new OkHttpClient.Builder()
                .connectTimeout(3600, TimeUnit.SECONDS)
                .readTimeout(3600, TimeUnit.SECONDS)
                .writeTimeout(3600, TimeUnit.SECONDS)
                .build();
        Request request = new Request.Builder()
                .url("http://" + Constants.controllerIP + "/passadeiras?skill=" + skillID)
                .build();
        try {
            kitt_library.kittBeingUsed.acquire();
            Thread.sleep(100);
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }
        try (Response response = client.newCall(request).execute()) {
            // Body automatically closed by try-with-resources
        }
        kitt_library.kittBeingUsed.release();
    }

    public static void executeStation(String skillID) throws IOException {
        OkHttpClient client = new OkHttpClient.Builder()
                .connectTimeout(3600, TimeUnit.SECONDS)
                .readTimeout(3600, TimeUnit.SECONDS)
                .writeTimeout(3600, TimeUnit.SECONDS)
                .build();
        Request request = new Request.Builder()
                .url("http://" + Constants.controllerIP + "/estacao?skill=" + skillID)
                .build();
        try {
            kitt_library.kittBeingUsed.acquire();
            Thread.sleep(100);
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }
        try (Response response = client.newCall(request).execute()) {
            // Body automatically closed by try-with-resources
        }
        kitt_library.kittBeingUsed.release();
    }

    public static void main(String[] args) throws IOException, InterruptedException {
        for (int i = 0; i < 2; i++) {
            kitt_library.executeTransporte("AB");
            Thread.sleep(1000);
            kitt_library.executeTransporte("BC");
            Thread.sleep(1000);
            kitt_library.executeTransporte("CD");
            Thread.sleep(1000);
            kitt_library.executeStation("0");
            Thread.sleep(1000);
            kitt_library.executeStation("1");
            Thread.sleep(1000);
            kitt_library.executeTransporte("DE");
            Thread.sleep(1000);
            kitt_library.executeTransporte("EF");
            Thread.sleep(1000);
            kitt_library.executeTransporte("FA");
            Thread.sleep(1000);
        }
    }
}
