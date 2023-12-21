/* Adilet Kuroda 
 * Date: 04/17/2023
 * Description: This class implements the Runnable interface and overrides 
 * the run method. The run method will be called when the thread is started.
 * It will send the current date/time to the client after 5 seconds.
 * 
 */

import java.net.*; // for Socket
import java.io.*; // for IOException and Input/OutputStream

// Worker class implements the Runnable interface
class Worker implements Runnable {
    // private Socket client variable that will hold the client socket
    private Socket client;
    // constructor that takes a Socket object as a parameter
    public Worker(Socket client) {
        this.client = client;
    }
    // run method of the Runnable interface implementation
    // this method will be called when the thread is started
    // It will send the current date/time to the client after 5 seconds
    // writing the message to the client socket
    @Override
    public void run() {
        try {
            // sleep for 5 seconds
            Thread.sleep(5000);
            // get the output stream of the client socket
            PrintWriter out = new PrintWriter(client.getOutputStream(), true);
            // send the current date/time to the client
            out.println(new java.util.Date().toString());
            // close the client socket
            client.close();
            // catch exceptions and print the stack trace
        } catch (IOException e) {
            e.printStackTrace();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}