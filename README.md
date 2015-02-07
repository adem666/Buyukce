# Buyukce
Buyukce is a library for android, to create big image without using java or c++ heap


first create big bitmap in a temp folder

```java
int width=10000;
int height=10000;
Buyukce big = Buyukce.createBigBitmap(ctx, "/sdcard/tmp", width, height);
```

then you can draw object on this object

```java
// you can set target rectangle. and also you can set strech or not
big.drawBitmap(bitmap, new Rect(0,0,100,100), true);
```

you can draw ui object too

```java
TextView text = ....
big.drawView(text, new Rect(0,100,100,100));
```

at last, you can export this image.

```java
String destinationFile = ...
// you can set format. but for now only JPEG is supported. PNG will be supported too
// you can set quality between 0-1
boolean result = big.export(destinationFile, Buyukce.Format.JPEG, 0.8);
```

