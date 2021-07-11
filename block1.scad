//
// Block to hold LEDs and servo
//
//

h = 15;
th = 1.7;
w = 43;
wy = 33;
wc = 5;
ser_x = 23;
ser_y = 12.5;

module pre()
{
    cube([w, wy, h], center=true);
}

module rem()
{
    translate([w/2-wc/2+0.1, 0, h/2-th/2-11])
    cube([5, 40, th], center=true);

    translate([-w/2+ser_x/2+4, -wy/2+ser_y/2+4, 0])
    cube([ser_x, ser_y, 50], center=true);
}

module block()
{
    difference()
    {
        pre();
        rem();
    }
}


block();

