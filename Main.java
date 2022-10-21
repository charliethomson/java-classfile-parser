import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.METHOD)
@interface Test {
    int age();
    double netWorth();
}

class ParentClass {
    int test3(int a) { return a; }
}
interface TestInterface {
    int test4(int a);
}
public class Main extends ParentClass implements TestInterface {
    public int test4(int a) { return a; }
    @Override
    int test3(int a) { return a * a; }

    @Deprecated
    private String thisIsAField;

    /**
    * Returns an Image object that can then be painted on the screen.
    * The url argument must specify an absolute <a href="#{@link}">{@link URL}</a>. The name
    * argument is a specifier that is relative to the url argument.
    * <p>
    * This method always returns immediately, whether or not the
    * image exists. When this applet attempts to draw the image on
    * the screen, the data will be loaded. The graphics primitives
    * that draw the image will incrementally paint on the screen.
    *
    * @param  url  an absolute URL giving the base location of the image
    * @param  name the location of the image, relative to the url argument
    * @return      the image at the specified URL
    * @see         Image
    */
    @Deprecated
    @Test(age=10, netWorth=200d)
    public void getImage(String url, String name) {
    }

    private String[] thisIsAnArray;

    public static int test(int a, int b) {
        return a + b;
    }
    public static int test2(int[] a, boolean cont) {
        int b = 0;
        for (int c : a) b += c;
        return b;
    }

    public static void main(String[] args) {
        System.out.println(test(10, 12));
    }
}