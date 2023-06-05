use <mattwach/util.scad>
include <heat_sink.scad>
include <main_pcb.scad>
include <NopSCADlib/core.scad>
include <NopSCADlib/vitamins/fans.scad>

module cooling_fan() {
  fan(fan120x25);  
}

module case() {
  //heat_sink();
  //cooling_fan();
  main_pcb();
}


$fa=2.0;
$fs=0.5;
case();
