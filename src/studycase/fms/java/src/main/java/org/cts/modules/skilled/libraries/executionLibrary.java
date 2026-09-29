package org.cts.modules.skilled.libraries;

import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;
import org.cts.utilities.Constants;

import java.io.IOException;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

public class executionLibrary {

    public static Semaphore hardwareBeingUsed = new Semaphore(1);

    public static void executeSkill(String skillURL) throws IOException {
        OkHttpClient client = new OkHttpClient.Builder()
                .connectTimeout(30, TimeUnit.SECONDS)
                .readTimeout(30, TimeUnit.SECONDS)
                .writeTimeout(30, TimeUnit.SECONDS)
                .build();
        Request request = new Request.Builder()
                .url(skillURL)
                .build();
        try {
            // hardwareBeingUsed.acquire();
            Thread.sleep(100);
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }
        try (Response response = client.newCall(request).execute()) {
            // Body automatically closed by try-with-resources
        } catch (IOException e) {
            System.err.println("[Simulation] Hardware unreachable at " + skillURL + " - executed skill in simulation mode.");
        }
        // hardwareBeingUsed.release();
    }

}
