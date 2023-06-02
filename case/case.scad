use <mattwach/util.scad>
include <heat_sink.scad>
include <main_pcb.scad>

module case() {
  //heat_sink();
  main_pcb();
}

$fa=2.0;
$fs=0.5;
case();
