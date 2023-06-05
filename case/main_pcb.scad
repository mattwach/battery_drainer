use <mattwach/util.scad>
include <mattwach/vitamins/electronics/buttons.scad>
include <mattwach/vitamins/electronics/fuse.scad>
include <mattwach/vitamins/electronics/oled.scad>
include <mattwach/vitamins/electronics/pi_pico.scad>
include <mattwach/vitamins/rc/xt60.scad>
include <NopSCADlib/vitamins/pin_headers.scad>

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

  module xt60_connector() {
    xt60_xinset = 18.4;
    xt60_yinset = 10.5;
    translate([
        main_pcb_length - xt60_xinset,
        xt60_yinset,
        main_pcb_thickness]) xt60_pw_male();
  }

  module buttons() {
    button_width = 3.6;
    module button() {
      tz(main_pcb_thickness) pcb_button(
          length = 6.2,
          width = button_width,
          height = 3.5,
          button_length = 3,
          button_width = 1.5,
          button_height = 2
      );
    }

    module reset_button() {
      reset_button_xoffset = 25;
      reset_button_yoffset = 46;
      txy(reset_button_xoffset, reset_button_yoffset) button();
      
    }

    module control_buttons() {
      control_button_xoffset = 66;
      control_button_yoffset = 11.4;
      control_button_count = 4;
      control_button_xspan = 10;

      for (i = [0:control_button_count-1]) {
        txy(
            button_width + control_button_xoffset + i * control_button_xspan,
            control_button_yoffset) rz(90) button();
      }
    }

    reset_button();
    control_buttons();
  }

  module fuse() {
    fuse_xoffset = 7.2;
    fuse_yoffset = 32;
    fuse_zoffset = 7 + main_pcb_thickness;
    translate([
        main_pcb_length - fuse_xoffset,
        fuse_yoffset,
        fuse_zoffset]) rx(-90) glass_fuse(diameter=5.6, length=32, metal_length=6.4);
  }

  module pins() {
    module fan_pins() {
      pin_centerx = 2.54 * 2;
      pin_centery = 2.54 / 2;
      translate([
          pin_centerx,
          pin_centery,
          main_pcb_thickness]) pin_header(2p54header, 4, 1);
    }

    fan_pins();
  }

  pcb();
  pico();
  oled();
  xt60_connector();
  buttons();
  fuse();
  pins();
}

