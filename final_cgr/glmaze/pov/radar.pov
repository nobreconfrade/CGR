// Persistence Of Vision config file
// Till Harbaum

global_settings { assumed_gamma 2.2 }

camera {
   location  <0, 0, -2.3>
   direction <0, 0, 1>
   look_at   <0, 0, 0>

   up        <0,1,0> 
   right     <2,0,0>
}

background { color red 0 green 0 blue 0 }

object {
        torus { 1, .1 
           finish { ambient 0 diffuse 1 phong 0 }
	   pigment { color red 1 green 1 blue 1 } 
	   rotate <90, 0, 0>
	}
}

object {
        torus { 2, .1 
           finish { ambient 0 diffuse 1 phong 0 }
	   pigment { color red 1 green 1 blue 1 } 
	   rotate <90, 0, 0>
	   translate <0, -1, 0>
	}
}

light_source { <0, 0, -20> color red 1 green 1 blue 1 }
