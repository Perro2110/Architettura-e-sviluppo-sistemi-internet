// 17.35 17.46
import java.io.*;
import java.net.*;

public class Main {
    public static void main(String[] args) {
        if (args.length != 2) {
            System.err.println("Numero argomenti sbagliato \n");
            System.exit(1);
        }
        try (Socket theSocket = new Socket(args[0], Integer.parseInt(args[1]))) {
            BufferedReader userIn = new BufferedReader(new InputStreamReader(System.in));
            BufferedReader networkIn = new BufferedReader(new InputStreamReader(theSocket.getInputStream(), "UTF-8"));
            BufferedWriter networkOut = new BufferedWriter(
                    new OutputStreamWriter(theSocket.getOutputStream(), "UTF-8"));

            for (;;) {
                // MANDO AL SERVER
                String insUtente;
                System.out.println("Inserire categoria dei regali,'fine' per finire");
                insUtente = userIn.readLine();
                networkOut.write(insUtente);
                networkOut.newLine();
                networkOut.flush();
                if (insUtente.equals("fine"))
                    break;

                System.out.println("Inserire nome del produttore dei regali,'fine' per finire");
                insUtente = userIn.readLine();
                networkOut.write(insUtente);
                networkOut.newLine();
                networkOut.flush();
                if (insUtente.equals("fine"))
                    break;

                System.out.println("Inserire modalità di ordinamento dei regali, 'crescente' o 'decrescente, 'fine' per finire");
                insUtente = userIn.readLine();
                networkOut.write(insUtente);
                networkOut.newLine();
                networkOut.flush();
                if (insUtente.equals("fine"))
                    break;
                // RICEVO DAL SERVER
                String theLine;
                while((theLine=networkIn.readLine())!=null)
                {
                    if(theLine.trim().equals("==fin=="))
                        break;
                    System.out.println(theLine);
                }
            }
            networkIn.close();
            networkOut.close();
            userIn.close();
            // CHIUDO CONNESSIONE
        } catch (Exception e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(2);
        }
    }
}
