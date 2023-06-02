use <mattwach/util.scad>
include <mattwach/vitamins/electronics/oled.scad>
include <mattwach/vitamins/electronics/pi_pico.scad>

module main_pcb() {
  overlap = 0.01;
  main_pcb_length = 121.9;
  main_pcb_width = 88;
  main_pcb_thickness = 1.8;

  module pcb() {
    module mount_holes() {
      holes_xspan = 115;
      holes_yspan = 79.8;
      hole_diameter = 3.2;

      module hole() {
        tz(-overlap) cylinder(d=hole_diameter, h=main_pcb_thickness + overlap * 2);
      }

      module side() {
        txy((main_pcb_length - holes_xspan) / 2, (main_pcb_width - holes_yspan) / 2) hole();
        txy((main_pcb_length - holes_xspan) / 2 + holes_xspan, (main_pcb_width - holes_yspan) / 2) hole();
      }

      side();
      ty(holes_yspan) side();
    }

    module board() {
      color("#070") cube([
          main_pcb_length,
          main_pcb_width,
          main_pcb_thickness]);
    }

    difference() {
      board();
      mount_holes();
    }
  }

  module pico() {
    pico_x_offset = 0.5;
    pico_y_offset = 11;
    translate([
        pico_x_offset,
        pico_y_offset,
        main_pcb_thickness]) pi_pico();
  }

  module oled() {
    oled_x_offset = 65.6;
    oled_y_offset = 23;
    oled_z_offset = main_pcb_thickness + 2.54;
    translate([
        oled_x_offset,
        oled_y_offset,
        oled_z_offset]) oled_128x64_1_5in_i2c(true);
  }

  pcb();
  pico();
  oled();
}

