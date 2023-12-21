/**Adilet Kuroda 
 * Date: 04/24/2023
 * This class passes instance of Runnable to the Thread constructor that 
 * is implementd in the Worker class to Executer Service class. Executer Service
 * has a thread pool that will manage the threads and reuse them when they are 
 * done. The thread pool is fixed to 20 threads. The thread pool will manage the
 * threads and reuse them when they are done. This way, it allows the server to
 * create fixed number of threads and reuse them when they are done. 
 */

import java.net.*;
import java.util.concurrent.ExecutorService;
import java.io.*;
import static java.util.concurrent.Executors.newFixedThreadPool;

// DateServerMTP class that will create a server socket and listen for incoming
// connections on the specified port. When a connection is received, it will
// create a new thread and submit it to a thread pool to handle the client request.
// The thread pool will manage the threads and reuse them when they are done.
public class DateServerMTP {
    public static void main(String[] args) {
        // check if a port number was provided
        if (args.length <= 0) {
            System.err.println("ERROR: No port provided");
            System.exit(1);
        }
        // To store the port number
        int port = 0;
        // parse the port number and store it in an integer
        try {
            port = Integer.parseInt(args[0]);
            // testing  purposes
            // System.out.println("Port: " + port);
        } catch (NumberFormatException e) {
            System.err.println("ERROR: value provided is not a number");
            System.exit(1);
        }
        try {
            // create a thread pool with 20 threads
            ExecutorService pool = newFixedThreadPool(20);
            // create a new server socket. Try-with-resources will automatically
            // close the socket when the try block is exited
            try (ServerSocket serverSocket = new ServerSocket(port)) {
                // continuously listen for incoming connections
                while (true) {
                    Socket client = serverSocket.accept();
                    // create a new worker to handle the client request
                    // and submit it to the thread pool
                    pool.submit(new Worker(client));
                }
            }
        // catch exceptions  and print stack trace
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
