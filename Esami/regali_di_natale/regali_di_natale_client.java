import java.io.*;
import java.net.*;

public class Main {
    public static void main(String[] args) {
        if (args.length != 2) {
            System.err.println("Errore di argomenti");
            System.exit(1);
        }
        try (Socket theSocket = new Socket(args[0], Integer.parseInt(args[1]))) {
            BufferedReader userIn = new BufferedReader(new InputStreamReader(System.in));
            BufferedReader networkIn = new BufferedReader(new InputStreamReader(theSocket.getInputStream(), "UTF-8"));
            BufferedWriter networkOut = new BufferedWriter(
                    new OutputStreamWriter(theSocket.getOutputStream(), "UTF-8"));

            // mando al server
            for (;;) {
                System.out.println("Inserisci nome utente");
                String nome = userIn.readLine();
                networkOut.write(nome);
                networkOut.newLine();
                networkOut.flush();
                if (nome.equals("fine"))
                    break;

                System.out.println("Inserisci password utente");
                String password = userIn.readLine();
                networkOut.write(password);
                networkOut.newLine();
                networkOut.flush();
                if (password.equals("fine"))
                    break;

                System.out.println("Inserisci categoria utente");
                String categoria = userIn.readLine();
                networkOut.write(categoria);
                networkOut.newLine();
                networkOut.flush();
                if (categoria.equals("fine"))
                    break;

                // Lettura risultati
                String theLine;
                while ((theLine = networkIn.readLine()) != null) {
                    if (theLine.trim().equals("==fin=="))
                        break;
                    System.out.println(theLine);
                }
            }
            networkIn.close();
            networkOut.close();
            userIn.close();
        } catch (Exception e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(2);
        }
    }
}
