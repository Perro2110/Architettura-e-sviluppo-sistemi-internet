import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.ServerSocket;
import java.net.Socket;

public class RemoteSquareServer {
   public RemoteSquareServer() {
   }

   public static void main(String[] var0) {
      if (var0.length != 1) {
         System.err.println("Errore! Sintassi corretta \u00e8:");
         System.err.println("\tjava RemoteSquareServer porta");
         System.exit(1);
      }

      String var1 = var0[0];

      try {
         ServerSocket var2 = new ServerSocket(Integer.parseInt(var1));
         Socket var3 = var2.accept();
         BufferedReader var4 = new BufferedReader(new InputStreamReader(var3.getInputStream(), "UTF-8"));
         BufferedWriter var5 = new BufferedWriter(new OutputStreamWriter(var3.getOutputStream(), "UTF-8"));
         String var6 = var4.readLine();
         System.out.println("numero arrivato: " + var6);
         Integer var8;
         while(!("fine".equals(var6))){
            var8 = (Integer.parseInt(var6));
            var8 *= var8;
            var5.write((var8) + "\n");
            var5.flush();
            var6 = var4.readLine();
            System.out.println("numero arrivato: " + var6);
         }
         var5.flush();
         var3.close();
      } catch (Exception var10) {
         System.err.println(var10.getMessage());
         var10.printStackTrace(System.err);
         System.exit(1);
      }

   }
}
