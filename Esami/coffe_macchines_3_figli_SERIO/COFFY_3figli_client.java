import java.io.*;
import java.net.*;

// 12.56;

public class client {
    public static void main(String[] args) {
        if (args.length != 2) {
            System.err.println("NUMERO ARGOMENTI");
            System.exit(1);
        }
        try (Socket thSocket = new Socket(args[0], Integer.parseInt(args[1]))) {
            BufferedReader userIn = new BufferedReader(new InputStreamReader(System.in));
            BufferedReader networkIn = new BufferedReader(new InputStreamReader(thSocket.getInputStream(), "UTF-8"));
            BufferedWriter networkOut = new BufferedWriter(new OutputStreamWriter(thSocket.getOutputStream(), "UTF-8"));

            for (;;) {
                String insUtente;
                System.out.println("Inserire nome utente, 'fine' per terminare");
                insUtente=userIn.readLine();
                networkOut.write(insUtente);
                networkOut.newLine();
                networkOut.flush();
                if (insUtente.equals("fine"))
                    break;

                System.out.println("Inserire nome utente, 'fine' per terminare");
                insUtente=userIn.readLine();
                networkOut.write(insUtente);
                networkOut.newLine();
                networkOut.flush();
                if (insUtente.equals("fine"))
                    break;

                System.out.println("Inserire nome utente, 'fine' per terminare");
                insUtente=userIn.readLine();
                networkOut.write(insUtente);
                networkOut.newLine();
                networkOut.flush();
                if (insUtente.equals("fine"))
                    break;
                
                String theLine;
                while((theLine = networkIn.readLine())!=null)
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
