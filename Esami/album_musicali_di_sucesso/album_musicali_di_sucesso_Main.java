//11.56 12.05 
import java.io.*;
import java.net.*;

public class Main {
    public static void main(String[] args) {
        if (args.length != 2) {
            System.err.println("NUMERO ARGOMENTI");
            System.exit(1);
        }
        try (Socket theSocket = new Socket(args[0], Integer.parseInt(args[1]))) {
            BufferedReader userIn = new BufferedReader(new InputStreamReader(System.in));
            BufferedReader newtworkIn = new BufferedReader(new InputStreamReader(theSocket.getInputStream(), "UTF-8"));
            BufferedWriter newtworkOUT = new BufferedWriter(
                    new OutputStreamWriter(theSocket.getOutputStream(), "UTF-8"));

            for (;;) {
                String insUtente;
                System.out.println("Inserire USERNAME, 'fine' per terminare");
                insUtente = userIn.readLine();
                newtworkOUT.write(insUtente);
                newtworkOUT.newLine();
                newtworkOUT.flush();
                if (insUtente.equals("fine"))
                    break;

                System.out.println("Inserire PASSWORD, 'fine' per terminare");
                insUtente = userIn.readLine();
                newtworkOUT.write(insUtente);
                newtworkOUT.newLine();
                newtworkOUT.flush();
                if (insUtente.equals("fine"))
                    break;

                System.out.println("Inserire NOME CANTANTE, 'fine' per terminare");
                insUtente = userIn.readLine();
                newtworkOUT.write(insUtente);
                newtworkOUT.newLine();
                newtworkOUT.flush();
                if (insUtente.equals("fine"))
                    break;

                System.out.println("Inserire COGNOME CANTANTE, 'fine' per terminare");
                insUtente = userIn.readLine();
                newtworkOUT.write(insUtente);
                newtworkOUT.newLine();
                newtworkOUT.flush();
                if (insUtente.equals("fine"))
                    break;
                
                String theLine;
                while((theLine = newtworkIn.readLine()) !=null)
                {
                    if(theLine.trim().equals("==fin=="))
                    break;
                    System.out.println(theLine);
                }
            }
            newtworkIn.close();
            newtworkOUT.close();
            userIn.close();

        } catch (Exception e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(2);
        }
    }
}
