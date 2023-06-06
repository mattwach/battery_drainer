use <../../util.scad>
include <../../../NopSCADlib/utils/core/core.scad>
include <../../../NopSCADlib/vitamins/pin_headers.scad>

module oled_128x64_i2c(include_pin_headers=true) {
  overlap = 0.01;

  bolt_ring_extrude = 0.1;
  cable_slot_depth = 1.5;
  pcb_length = 27.5;
  pcb_thickness= 1.2;
  pcb_width = 27.8;
  pin_hole_top_offset = 1.6;

  module pcb() {

    mount_hole_xspan = 23.5;
    mount_hole_yspan = 23.7;
    pin_hole_spacing = 2.54;

    module mount_hole() {
      mount_hole_diameter = 2;
      tz(-overlap - bolt_ring_extrude) cylinder(
          d=mount_hole_diameter, h=pcb_thickness + bolt_ring_extrude * 2 + overlap * 2);
    }

    module pin_hole() {
      pin_hole_diameter = 0.7;
      translate([
          pcb_length / 2,
          pcb_width - pin_hole_top_offset,
          -overlap
      ]) cylinder(d=pin_hole_diameter, h=pcb_thickness + overlap * 2);
    }

    module cable_slot() {
      cable_slot_length = 14;
      translate([
          (pcb_length - cable_slot_length) / 2,
          -overlap,
          -overlap
      ]) cube([
          cable_slot_length,
          cable_slot_depth + overlap,
          pcb_thickness + overlap * 2]);
    }

    module bolt_ring() {
      bolt_ring_diameter = 4;
      color("#ddd") tz(-bolt_ring_extrude) cylinder(
          d=bolt_ring_diameter, h=pcb_thickness + bolt_ring_extrude * 2); 
    }

    module board_with_bolt_rings() {
      difference() {
        color("#008") cube([
            pcb_length,
            pcb_width,
            pcb_thickness]);
        cable_slot();
      }
      txy((pcb_length - mount_hole_xspan) / 2,
          (pcb_width - mount_hole_yspan) / 2) bolt_ring();  
      txy((pcb_length - mount_hole_xspan) / 2 + mount_hole_xspan,
          (pcb_width - mount_hole_yspan) / 2) bolt_ring();  
      txy((pcb_length - mount_hole_xspan) / 2 + mount_hole_xspan,
          (pcb_width - mount_hole_yspan) / 2 + mount_hole_yspan) bolt_ring();
      txy((pcb_length - mount_hole_xspan) / 2,
          (pcb_width - mount_hole_yspan) / 2 + mount_hole_yspan) bolt_ring();
    }

    difference() {
      board_with_bolt_rings();
      txy((pcb_length - mount_hole_xspan) / 2,
          (pcb_width - mount_hole_yspan) / 2) mount_hole();  
      txy((pcb_length - mount_hole_xspan) / 2 + mount_hole_xspan,
          (pcb_width - mount_hole_yspan) / 2) mount_hole();  
      txy((pcb_length - mount_hole_xspan) / 2 + mount_hole_xspan,
          (pcb_width - mount_hole_yspan) / 2 + mount_hole_yspan) mount_hole();
      txy((pcb_length - mount_hole_xspan) / 2,
          (pcb_width - mount_hole_yspan) / 2 + mount_hole_yspan) mount_hole();
      tx(-pin_hole_spacing * 3 / 2) pin_hole();
      tx(-pin_hole_spacing / 2) pin_hole();
      tx(pin_hole_spacing / 2) pin_hole();
      tx(pin_hole_spacing * 3 / 2) pin_hole();
    }
  }

  module screen() {
    screen_top_offset = 4.7;
    oled_height = 15.5;
    oled_thickness = 0.7;

    module oled() {
      color("#222") translate([
          0,
          pcb_width - oled_height - screen_top_offset,
          pcb_thickness
      ]) cube([
        pcb_width,
        oled_height,
        oled_thickness
      ]);
    }

    module glass_cover() {
      glass_cover_height = 18.6;
      glass_cover_thickness = 2.8 - oled_thickness - pcb_thickness;
      color("#ddd", 0.1) translate([
          0,
          pcb_width - glass_cover_height - screen_top_offset,
          pcb_thickness + oled_thickness
      ]) cube([
        pcb_width,
        glass_cover_height,
        glass_cover_thickness
      ]);
    }

    module ribbon_cable() {
      ribbon_cable_width = 11;
      color("#322") union() {
        translate([
            (pcb_width - ribbon_cable_width) / 2,
            cable_slot_depth - overlap,
            pcb_thickness
        ]) cube([
          ribbon_cable_width,
          pcb_width - oled_height - screen_top_offset - cable_slot_depth,
          oled_thickness
        ]);
        translate([
            (pcb_width - ribbon_cable_width) / 2,
            cable_slot_depth - oled_thickness,
            0
        ]) cube([
          ribbon_cable_width,
          pcb_width - oled_height - screen_top_offset - cable_slot_depth,
          oled_thickness + pcb_thickness
        ]);
      }
    }

    oled();
    glass_cover();
    ribbon_cable();
  }

  module pin_headers() {
    translate([
        pcb_length / 2,
        pcb_width - pin_hole_top_offset,
        0
    ]) ry(180) pin_header(2p54header, 4, 1);
  }

  pcb();
  screen();
  if (include_pin_headers) {
    pin_headers();
  }
}

module oled_128x64_1_5in_i2c(include_pin_headers=true) {
  overlap = 0.01;

  bolt_ring_extrude = 0.1;
  cable_slot_depth = 3.9;
  pcb_length = 35.6;
  pcb_thickness= 1.2;
  pcb_width = 33.5;
  pin_hole_top_offset = 2;

  module pcb() {

    mount_hole_xspan = 30;
    mount_hole_yspan = 28.5;
    pin_hole_spacing = 2.54;

    module mount_hole() {
      mount_hole_diameter = 3.5;
      tz(-overlap - bolt_ring_extrude) cylinder(
          d=mount_hole_diameter, h=pcb_thickness + bolt_ring_extrude * 2 + overlap * 2);
    }

    module pin_hole() {
      pin_hole_diameter = 0.7;
      translate([
          pcb_length / 2,
          pcb_width - pin_hole_top_offset,
          -overlap
      ]) cylinder(d=pin_hole_diameter, h=pcb_thickness + overlap * 2);
    }

    module cable_slot() {
      cable_slot_length = 14;
      translate([
          (pcb_length - cable_slot_length) / 2,
          -overlap,
          -overlap
      ]) cube([
          cable_slot_length,
          cable_slot_depth + overlap,
          pcb_thickness + overlap * 2]);
    }

    module bolt_ring() {
      bolt_ring_diameter = 5;
      color("#ddd") tz(-bolt_ring_extrude) cylinder(
          d=bolt_ring_diameter, h=pcb_thickness + bolt_ring_extrude * 2); 
    }

    module board_with_bolt_rings() {
      difference() {
        color("#008") cube([
            pcb_length,
            pcb_width,
            pcb_thickness]);
        cable_slot();
      }
      txy((pcb_length - mount_hole_xspan) / 2,
          (pcb_width - mount_hole_yspan) / 2) bolt_ring();  
      txy((pcb_length - mount_hole_xspan) / 2 + mount_hole_xspan,
          (pcb_width - mount_hole_yspan) / 2) bolt_ring();  
      txy((pcb_length - mount_hole_xspan) / 2 + mount_hole_xspan,
          (pcb_width - mount_hole_yspan) / 2 + mount_hole_yspan) bolt_ring();
      txy((pcb_length - mount_hole_xspan) / 2,
          (pcb_width - mount_hole_yspan) / 2 + mount_hole_yspan) bolt_ring();
    }

    difference() {
      board_with_bolt_rings();
      txy((pcb_length - mount_hole_xspan) / 2,
          (pcb_width - mount_hole_yspan) / 2) mount_hole();  
      txy((pcb_length - mount_hole_xspan) / 2 + mount_hole_xspan,
          (pcb_width - mount_hole_yspan) / 2) mount_hole();  
      txy((pcb_length - mount_hole_xspan) / 2 + mount_hole_xspan,
          (pcb_width - mount_hole_yspan) / 2 + mount_hole_yspan) mount_hole();
      txy((pcb_length - mount_hole_xspan) / 2,
          (pcb_width - mount_hole_yspan) / 2 + mount_hole_yspan) mount_hole();
      tx(-pin_hole_spacing * 3 / 2) pin_hole();
      tx(-pin_hole_spacing / 2) pin_hole();
      tx(pin_hole_spacing / 2) pin_hole();
      tx(pin_hole_spacing * 3 / 2) pin_hole();
    }
  }

  module screen() {
    screen_top_offset = 5;
    oled_height = 18.3;
    oled_thickness = 0.7;

    module oled() {
      color("#222") translate([
          0,
          pcb_width - oled_height - screen_top_offset,
          pcb_thickness
      ]) cube([
        pcb_length,
        oled_height,
        oled_thickness
      ]);
    }

    module glass_cover() {
      glass_cover_height = 18.6;
      glass_cover_thickness = 2.8 - oled_thickness - pcb_thickness;
      color("#ddd", 0.1) translate([
          0,
          pcb_width - glass_cover_height - screen_top_offset,
          pcb_thickness + oled_thickness
      ]) cube([
        pcb_length,
        glass_cover_height,
        glass_cover_thickness
      ]);
    }

    module ribbon_cable() {
      ribbon_cable_width = 11;
      color("#322") union() {
        translate([
            (pcb_length - ribbon_cable_width) / 2,
            cable_slot_depth - overlap,
            pcb_thickness
        ]) cube([
          ribbon_cable_width,
          pcb_length - oled_height - screen_top_offset - cable_slot_depth,
          oled_thickness
        ]);
        translate([
            (pcb_length - ribbon_cable_width) / 2,
            cable_slot_depth - oled_thickness,
            0
        ]) cube([
          ribbon_cable_width,
          pcb_width - oled_height - screen_top_offset - cable_slot_depth,
          oled_thickness + pcb_thickness
        ]);
      }
    }

    oled();
    glass_cover();
    ribbon_cable();
  }

  module pin_headers() {
    translate([
        pcb_length / 2,
        pcb_width - pin_hole_top_offset,
        0
    ]) ry(180) pin_header(2p54header, 4, 1);
  }

  pcb();
  screen();
  if (include_pin_headers) {
    pin_headers();
  }
}

module oled_64x32_i2c(include_pin_headers=true) {
  overlap = 0.01;
  pcb_xsize = 15.2;
  pcb_ysize = 16;
  pcb_zsize = 1.2;
  pin_hole_spacing = 2.54;

  module pcb() {
    module board() {
      color("blue") cube([pcb_xsize, pcb_ysize, pcb_zsize]);
    }
    
    module pin_holes() {
      pin_count = 4;

      module hole() {
        pin_diameter = 0.9;
        pin_span = (pin_count - 1) * pin_hole_spacing;
        pin_xoffset = (pcb_xsize - pin_span) / 2;
        translate([
          pin_xoffset,
          pcb_ysize - pin_hole_spacing / 2,
          -overlap
        ]) cylinder(d=pin_diameter, h=pcb_zsize + overlap * 2);
      }

      for (i = [0:pin_count-1]) {
        tx(i * pin_hole_spacing) hole();
      }
    }

    difference() {
      board();
      pin_holes();
    }
  }

  module screen() {
    screen_xsize = 14.7;
    screen_ysize = 11.6;
    screen_zsize = 2.3 - pcb_zsize;
    
    screen_xoffset = (pcb_xsize - screen_xsize) / 2;
    screen_yoffset = 1.8;

    module glass_cover() {
      glass_cover_ysize = 8.5;
      glass_cover_thickness = 0.5;
      color("white", 0.2) translate([
          screen_xoffset,
          screen_yoffset + screen_ysize - glass_cover_ysize,
          pcb_zsize + screen_zsize]) cube([
            screen_xsize,
            glass_cover_ysize,
            glass_cover_thickness]);
    }

    color("#222") translate([
        screen_xoffset,
        screen_yoffset,
        pcb_zsize]) cube([screen_xsize, screen_ysize, screen_zsize]);
    glass_cover();
  }

  module pin_headers() {
    translate([
        pcb_xsize / 2,
        pcb_ysize - pin_hole_spacing / 2,
        0
    ]) ry(180) pin_header(2p54header, 4, 1);
  }

  pcb();
  screen();
  if (include_pin_headers) {
    pin_headers();
  }
}

/*
$fa=2;
$fs=0.5;
*oled_128x64_i2c();
oled_64x32_i2c();
*/

