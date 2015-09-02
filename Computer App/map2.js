// create a map with default options in an element with the id "map1"
var map = new OpenLayers.Map("mapdiv");
map.addLayer(new OpenLayers.Layer.OSM());

epsg4326 =  new OpenLayers.Projection("EPSG:4326"); //WGS 1984 projection
projectTo = map.getProjectionObject();

var lonLat = new
  OpenLayers.LonLat( -0.1279688 ,51.5077286 ).transform(epsg4326, projectTo);

var zoom=1;
map.setCenter (lonLat, zoom);

    var markers = new OpenLayers.Layer.Markers( "Markers" );
    map.addLayer(markers);

    var size = new OpenLayers.Size(21,25);
    var offset = new OpenLayers.Pixel(-(size.w/2), -size.h);
    var icon = new OpenLayers.Icon('img/marker.png', size, offset);
    markers.addMarker(new OpenLayers.Marker(new OpenLayers.LonLat(0,0).transform(epsg4326, projectTo),icon));

var a = new OpenLayers.Layer.Vector("Yokoko");
var layer = new OpenLayers.Layer.GML("Yokoko",
   "test.txt",
   { format: OpenLayers.Format.Text });
map.addLayer(layer);
map.zoomToMaxExtent();

var caca = new OpenLayers.Layer.GML("Overlayed", "test.txt",
  { format: OpenLayers.Format.Text });
map.addLayer(layer);
map.zoomToMaxExtent();
