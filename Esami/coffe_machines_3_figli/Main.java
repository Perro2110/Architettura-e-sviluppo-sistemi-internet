
// 10.44 10:53
import java.io.*;
import java.net.*;

public class Main {
    public static void main(String[] args) {
        if (args.length != 2) {
            System.err.println("errore numero argomenti");
            System.exit(1);
        }
        try (Socket theSocket = new Socket(args[0], Integer.parseInt(args[1]))) {
            BufferedReader userIn = new BufferedReader(new InputStreamReader(System.in));
            BufferedReader networkIn = new BufferedReader(new InputStreamReader(theSocket.getInputStream(), "UTF-8"));
            BufferedWriter networkOut = new BufferedWriter(
                    new OutputStreamWriter(theSocket.getOutputStream(), "UTF-8"));

            // mando al server
            for (;;) {
                System.out.println("Inserire username, 'fine' per terminare ");
                String nome = userIn.readLine();
                networkOut.write(nome);
                networkOut.newLine();
                networkOut.flush();
                if (nome.equals("fine"))
                    break;

                System.out.println("Inserire password, 'fine' per terminare ");
                String pass = userIn.readLine();
                networkOut.write(pass);
                networkOut.newLine();
                networkOut.flush();
                if (pass.equals("fine"))
                    break;

                System.out.println("Inserire categoria, 'fine' per terminare ");
                String cat = userIn.readLine();
                networkOut.write(cat);
                networkOut.newLine();
                networkOut.flush();
                if (cat.equals("fine"))
                    break;

                // leggo dal server
                String theLine;
                while((theLine = networkIn.readLine())!=null){
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
