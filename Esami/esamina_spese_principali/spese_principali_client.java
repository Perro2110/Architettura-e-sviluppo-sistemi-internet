import java.io.*;
import java.net.*;


public class Main {
    public static void main(String args[]){
        if(args.length != 2){
            System.err.println("Errore! numero di argomenti");
            System.exit(1);
        }
        try (Socket theSocket = new Socket(args[0],Integer.parseInt(args[1]))){
            BufferedReader userIn = new BufferedReader(new InputStreamReader(System.in));
            BufferedReader networkIn = new BufferedReader(new InputStreamReader(theSocket.getInputStream(),"UTF-8"));
            BufferedWriter networkOut = new BufferedWriter(new OutputStreamWriter(theSocket.getOutputStream(),"UTF-8"));

           
            for(;;)
            { 
                // MANDO AL SERVER
                System.out.println("inserire str1: ");
                String str1 = userIn.readLine();
                networkOut.write(str1);
                networkOut.newLine();
                networkOut.flush();
                if (str1.equals("fine")){
                    break;
                }

                System.out.println("inserire str2: ");
                String str2 = userIn.readLine();
                networkOut.write(str2);
                networkOut.newLine();
                networkOut.flush();
                if (str2.equals("fine")){
                    break;
                }

                System.out.println("inserire str3: ");
                String str3 = userIn.readLine();
                networkOut.write(str3);
                networkOut.newLine();
                networkOut.flush();
                if (str3.equals("fine")){
                    break;
                }
                // lettura server
                String theLine;
                while ((theLine = networkIn.readLine()) != null){
                    if(theLine.trim().equals("===fin===")){
                        break;
                    }
                    System.out.println(theLine);
                }
            }
            networkIn.close();
            networkOut.close();
            userIn.close();
            theSocket.close();
        }catch (Exception e) {
           System.err.println(e.getMessage());
           e.printStackTrace();
           System.exit(2);
        }
    }    
}
