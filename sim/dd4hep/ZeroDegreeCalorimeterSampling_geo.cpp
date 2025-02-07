// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong

#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/OpticalSurfaces.h"
#include "DD4hep/Printout.h"
#include "DDRec/DetectorData.h"
#include "DDRec/Surface.h"
#include <XML/Helper.h>
#include <XML/Layering.h>
//////////////////////////////////////////////////
// Far Forward Ion Zero Degree Calorimeter - Hcal
//////////////////////////////////////////////////

using namespace std;
using namespace dd4hep;

static Ref_t createDetector(Detector& desc, xml_h e, SensitiveDetector sens)
{
  xml_det_t x_det   = e;
  string    detName = x_det.nameStr();
  int       detID   = x_det.id();

  xml_dim_t dim   = x_det.dimensions();
  double    Width = dim.x();
  // double     Length     = dim.z();

  xml_dim_t pos = x_det.position();
  xml_dim_t rot = x_det.rotation();

  Material Vacuum = desc.material("Vacuum");

  double totWidth = Layering(x_det).totalThickness();
	printout(WARNING, "layer total thickness: ", to_string(totWidth));

  Box    envelope(Width / 2.0, Width / 2.0, totWidth / 2.0);
  Volume envelopeVol(detName + "_envelope", envelope, Vacuum);
  envelopeVol.setVisAttributes(desc.visAttributes(x_det.visStr()));
  PlacedVolume pv;

  static int layer_num = 1;
  double z = 0;
  // Read layers
  for (xml_coll_t c(x_det, _U(layer)); c; ++c) {
    xml_comp_t x_layer    = c;
    int        repeat     = x_layer.repeat();
    double     layerWidth = 0;

    for (xml_coll_t l(x_layer, _U(slice)); l; ++l)
      layerWidth += xml_comp_t(l).thickness();

    // Loop over repeat#
    for (int i = 0; i < repeat; i++) {
      double zlayer     = 0;
      string layer_name = detName + _toString(layer_num, "_layer%d");
      Volume layer_vol(layer_name, Box(Width / 2.0, Width / 2.0, layerWidth / 2.0), Vacuum);

      int slice_num = 1;
      // Loop over slices
      for (xml_coll_t l(x_layer, _U(slice)); l; ++l) {
        xml_comp_t x_slice    = l;
        double     w          = x_slice.thickness();
        string     slice_name = layer_name + _toString(slice_num, "_slice%d");
        Material   slice_mat  = desc.material(x_slice.materialStr());
        Volume     slice_vol(slice_name, Box(Width / 2.0, Width / 2.0, w / 2.0), slice_mat);

        if (x_slice.isSensitive()) {
          sens.setType("calorimeter");
          slice_vol.setSensitiveDetector(sens);
        }

        slice_vol.setAttributes(desc, x_slice.regionStr(), x_slice.limitsStr(), x_slice.visStr());
        pv = layer_vol.placeVolume(
            slice_vol, Transform3D(RotationZYX(0, 0, 0), Position(0.0, 0.0, zlayer - layerWidth / 2.0 + w / 2.0)));
        pv.addPhysVolID("slice", slice_num);
        zlayer += w;
        ++slice_num;
      }

      string layer_vis = dd4hep::getAttrOrDefault<std::string>(x_layer, _Unicode(vis), "InvisibleWithDaughters");
      layer_vol.setAttributes(desc, x_layer.regionStr(), x_layer.limitsStr(), layer_vis);
      pv = envelopeVol.placeVolume(
          layer_vol,
          Transform3D(RotationZYX(0, 0, 0), Position(0, 0, z - totWidth / 2.0 + layerWidth / 2.0)));
      pv.addPhysVolID("layer", layer_num);
			z += layerWidth;
      ++layer_num;
    }
  }

  DetElement   det(detName, detID);
  Volume       motherVol = desc.pickMotherVolume(det);
  Transform3D  tr(RotationZYX(rot.z(), rot.y(), rot.x()), Position(pos.x(), pos.y(), pos.z() + totWidth / 2.0));
  PlacedVolume phv = motherVol.placeVolume(envelopeVol, tr);
  phv.addPhysVolID("system", detID);
  det.setPlacement(phv);

  return det;
}

static Ref_t createPolyhedraZDC(Detector& desc, xml_h e, SensitiveDetector sens)
{
  xml_det_t x_det   = e;
  string    detName = x_det.nameStr();
  int       detID   = x_det.id();

  xml_dim_t dim   = x_det.dimensions();
  double    width = dim.x();
	double		height = dim.y();
  double    det_thickness = dim.z();

  xml_dim_t pos = x_det.position();
  xml_dim_t rot = x_det.rotation();

  Material Vacuum = desc.material("Vacuum");

  // double det_thickness = Layering(x_det).totalThickness();

  Box    envelope(width / 2.0, height / 2.0, det_thickness / 2.0);
  Volume envelopeVol(detName + "_envelope", envelope, Vacuum);
  envelopeVol.setVisAttributes(desc.visAttributes(x_det.visStr()));
  PlacedVolume pv;

  static int layer_num = 1;
  double z = 0;
  // Read layers
  for (xml_coll_t c(x_det, _U(layer)); c; ++c) {
    xml_comp_t x_layer    = c;
    int        repeat     = x_layer.repeat();
    double     layerThickness = 0;

    for (xml_coll_t l(x_layer, "*"); l; ++l)
		{
			xml_comp_t x_comp = l;
			if (x_comp.hasAttr(_U(count)) && 0 == x_comp.count())
				continue;
			layerThickness += x_comp.thickness();
		}
		printout(WARNING, "layer thickness: ", to_string(layerThickness));

    // Loop over repeat#
    for (int i = 0; i < repeat; i++) {
      double zlayer     = 0;
      string layer_name = detName + _toString(layer_num, "_layer%d");
      Volume layer_vol(layer_name, Box(width / 2.0, height / 2.0, layerThickness / 2.0), Vacuum);

      int comp_num = 1;
			int sensitive_num = 1;
			int nonsensitive_num = 1000;
      // Loop over layer components
      for (xml_coll_t l(x_layer, "*"); l; ++l) {
        xml_comp_t x_comp		= l;
				string		 comp_tag = x_comp.tag();
        double     t				= x_comp.thickness();
        string     comp_name = layer_name + "_" + comp_tag + "_" + x_comp.nameStr();
        Material   comp_mat  = desc.material(x_comp.materialStr());
        Volume     comp_vol(comp_name);
				comp_vol.setMaterial(comp_mat);

				if ("slice" == comp_tag)
				{
					comp_vol.setSolid(Box(width/2.0, height/2.0, t/2.0));
				}
				else if ("Box" == comp_tag)
				{
					comp_vol.setSolid(Box(x_comp.x(width)/2.0, x_comp.y(height)/2.0, t/2.0));
				}
				else if ("PolyhedraRegular" == comp_tag)
				{
					comp_vol.setSolid(PolyhedraRegular(x_comp.numsides(), x_comp.rmin(0), x_comp.rmax(), t));
				}
				else if ("Tube" == comp_tag)
				{
					comp_vol.setSolid(Tube(x_comp.rmin(0), x_comp.rmax(), t));
				}

        if (x_comp.isSensitive()) {
          sens.setType("calorimeter");
          comp_vol.setSensitiveDetector(sens);
        }

				double x_offset = x_comp.x_offset(0); 
				double y_offset = x_comp.y_offset(0); 
				double angle = 0;
				if (x_comp.hasAttr(_U(angle)))
					angle = x_comp.angle();	// in degree

        comp_vol.setAttributes(desc, x_comp.regionStr(), x_comp.limitsStr(), x_comp.visStr());
        pv = layer_vol.placeVolume(
            comp_vol, Transform3D(RotationZYX(angle, 0, 0), Position(x_offset, y_offset, zlayer - layerThickness / 2.0 + t / 2.0)));

        if (x_comp.isSensitive()) {
					pv.addPhysVolID("slice", sensitive_num);
					++sensitive_num;
				} else {
					pv.addPhysVolID("slice", nonsensitive_num);
					++nonsensitive_num;
				}

        ++comp_num;
				if (x_comp.hasAttr(_U(count)) && 0 == x_comp.count())
					continue;
        zlayer += t;
      }

      string layer_vis = dd4hep::getAttrOrDefault<std::string>(x_layer, _Unicode(vis), "InvisibleWithDaughters");
      layer_vol.setAttributes(desc, x_layer.regionStr(), x_layer.limitsStr(), layer_vis);
      pv = envelopeVol.placeVolume(
          layer_vol,
          Transform3D(RotationZYX(0, 0, 0), Position(0, 0, z - det_thickness / 2.0 + layerThickness / 2.0)));
      pv.addPhysVolID("layer", layer_num);
			z += layerThickness;
      ++layer_num;
    }
  }

  DetElement   det(detName, detID);
  Volume       motherVol = desc.pickMotherVolume(det);
  Transform3D  tr(RotationZYX(rot.z(), rot.y(), rot.x()), Position(pos.x(), pos.y(), pos.z() + det_thickness / 2.0));
  PlacedVolume phv = motherVol.placeVolume(envelopeVol, tr);
  phv.addPhysVolID("system", detID);
  det.setPlacement(phv);

  return det;
}

DECLARE_DETELEMENT(ZDC_Sampling, createDetector)
DECLARE_DETELEMENT(Polyhedra_ZDC_Sampling, createPolyhedraZDC)

/* vim: set shiftwidth=2 softtabstop=2 tabstop=2: */
