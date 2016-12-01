// Persistence Of Vision config file
// Till Harbaum

global_settings { assumed_gamma 2.2 }

camera {
   location  <0, 0, -2.3>
   direction <0, 0, 1>
   look_at   <0, 0, 0>

   up        <0,1,0> 
   right     <1,0,0>
}

background { color red .2 green .3 blue .4 }

object {
	sphere { <0.0, 0.0, 0.0>, 1
           finish { ambient 0 diffuse 1 phong 0 }
	   pigment { 
      		image_map{ png "smiley.png" once } 
		translate <-.5, -.5, 0>
		scale <4,2,1>
      	   } 
	}
}

light_source { <0, 0, -20> color red 1 green 1 blue 1 }
