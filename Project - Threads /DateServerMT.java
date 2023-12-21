/** Adilet Kuroda 
 * Date: 04/24/2023
 *  DateServerMT class that will create a server socket and listen for incoming
 * requests on the specified port. When a connection is received, it will create
 * a new thread per client request and handle the request. The server will
 * continue to listen for incoming connections until it is terminated.
 * When it is terminated, it closes the server socket with try-with-resources
 * to ensure that the ServerSocket is closed even if an exception occurs or 
 * the server is terminated. This class creates a new thread for each client
 * request. The thread is created by passing a Runnable object to the Thread
 * constructor. The runnable object is an instance of the Worker class.
 */

import java.net.*;
import java.io.*;

// DateServerMT class that will create a server socket and listen for incoming
// connections on the specified port. When a connection is received, it will
// create a new thread to handle the client request.
public class DateServerMT {
    public static void main(String[] args) {
        // check if a port number was provided
        if (args.length <= 0) {
            System.err.println("ERROR: No port provided");
            System.exit(1);
        }
        int port = 0;
        // parse the port number and store it in an integer
        try {
            port = Integer.parseInt(args[0]);
            // System.out.println("Port: " + port);
        } catch (NumberFormatException e) {
            System.err.println("ERROR: value provided is not a number");
            System.exit(1);
        }
        try {
            // create a new server socket. Try-with-resources will automatically
            // close the socket when the try block is exited
            try (ServerSocket serverSocket = new ServerSocket(port)) {
                // continuously listen for incoming connections
                while (true) {
                    // accept the incoming connection and store it in a Socket
                    Socket client = serverSocket.accept();
                    // create a new thread to handle the client request
                    Thread thread = new Thread(new Worker(client));
                    // start the thread by calling the run method
                    thread.start();
                }
            }
        // catch exceptions and print the stack trace
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}