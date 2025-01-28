// MARCO PERROTTA 184789
import java.io.*;
import java.net.*;

public class MARCO_PERROTTA_client {
    public static void main(String[] args) {
        if (args.length != 2) {
            System.err.println("Numero argomenti errato !!");
            System.exit(1);
        }
        try (Socket theSocket = new Socket(args[0], Integer.parseInt(args[1]))) {
            BufferedReader userIn = new BufferedReader(new InputStreamReader(System.in));
            BufferedReader networkIn = new BufferedReader(new InputStreamReader(theSocket.getInputStream(), "UTF-8"));
            BufferedWriter networkOut = new BufferedWriter(
                    new OutputStreamWriter(theSocket.getOutputStream(), "UTF-8"));

            for (;;) {
                String insUtente;

                System.out.println("Inserire nome cliente di interese, 'fine' per terminare");
                insUtente = userIn.readLine();
                networkOut.write(insUtente);
                networkOut.newLine();
                networkOut.flush();
                if (insUtente.equals("fine"))
                    break;

                System.out.println("Inserire numero N di interese, 'fine' per terminare");
                insUtente = userIn.readLine();
                networkOut.write(insUtente);
                networkOut.newLine();
                networkOut.flush();
                if (insUtente.equals("fine"))
                    break;

                System.out.println("Inserire data di interese, 'fine' per terminare");
                insUtente = userIn.readLine();
                networkOut.write(insUtente);
                networkOut.newLine();
                networkOut.flush();
                if (insUtente.equals("fine"))
                    break;
                
                String theLine;
                while((theLine = networkIn.readLine()) != null)
                {
                    if(theLine.trim().equals("==fin=="))
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
