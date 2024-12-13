// javac client.java 
// java args[0] args[1]

import java.io.*;
import java.net.*;

public class client {
    public static void main(String args[]) throws InterruptedException {
        if (args.length != 2) {
            System.err.println("Errore! La sintassi corretta è: java client hostname porta");
            System.exit(1);
        }
        try (Socket theSocket = new Socket(args[0], Integer.parseInt(args[1]))) {
            BufferedReader userIn = new BufferedReader(new InputStreamReader(System.in));
            BufferedReader networkIn = new BufferedReader(new InputStreamReader(theSocket.getInputStream(), "UTF-8"));
            BufferedWriter networkOut = new BufferedWriter(new OutputStreamWriter(theSocket.getOutputStream(), "UTF-8"));
            
            // Mando al server
            for (;;) {
                System.out.println("Inserisci il nome utente: ");
                String nome = userIn.readLine();
                networkOut.write(nome);
                networkOut.newLine();
                networkOut.flush();
                if (nome.equals("fine"))
                    break;

                System.out.println("Inserisci password: ");
                String password = userIn.readLine();
                networkOut.write(password);
                networkOut.newLine();
                networkOut.flush();
                if (password.equals("fine"))
                    break;

                System.out.println("oggetto: ");
                String oggetto = userIn.readLine();
                networkOut.write(oggetto);
                networkOut.newLine();
                networkOut.flush();
                if (oggetto.equals("fine"))
                    break;

                // LETTURA RISULTATI 
                String theLine;
                while ((theLine = networkIn.readLine()) != null) {
                    if (theLine.trim().equals("==fin==")) {
                        break;
                    }
                    System.out.println(theLine);
                }
            }
            networkIn.close();
            networkOut.close();
            userIn.close();
            theSocket.close();
        } catch (IOException e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(2);
        }
    }
}