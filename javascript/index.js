import {
  Animated as RNAnimated,
  NativeModules,
  NativeEventEmitter,
  PermissionsAndroid,
} from 'react-native';

import {isAndroid} from './utils';
import MapView from './components/MapView';
import Light from './components/Light';
import PointAnnotation from './components/PointAnnotation';
import Annotation from './components/annotations/Annotation'; // eslint-disable-line import/no-cycle
import Callout from './components/Callout';
import UserLocation from './components/UserLocation'; // eslint-disable-line import/no-cycle
import Camera from './components/Camera';
import VectorSource from './components/VectorSource';
import ShapeSource from './components/ShapeSource';
import RasterSource from './components/RasterSource';
import ImageSource from './components/ImageSource';
import Images from './components/Images';
import FillLayer from './components/FillLayer';
import FillExtrusionLayer from './components/FillExtrusionLayer';
import HeatmapLayer from './components/HeatmapLayer';
import LineLayer from './components/LineLayer';
import CircleLayer from './components/CircleLayer';
import SymbolLayer from './components/SymbolLayer';
import RasterLayer from './components/RasterLayer';
import BackgroundLayer from './components/BackgroundLayer';
import locationManager from './modules/location/locationManager';
import offlineManager from './modules/offline/offlineManager';
import snapshotManager from './modules/snapshot/snapshotManager';
import MarkerView from './components/MarkerView';
import Animated from './utils/animated/Animated';
import AnimatedMapPoint from './utils/animated/AnimatedPoint';
import AnimatedShape from './utils/animated/AnimatedShape';
import AnimatedCoordinatesArray from './utils/animated/AnimatedCoordinatesArray';
import AnimatedExtractCoordinateFromArray from './utils/animated/AnimatedExtractCoordinateFromArray';
import AnimatedRouteCoordinatesArray from './utils/animated/AnimatedRouteCoordinatesArray';
import Style from './components/Style';
import Logger from './utils/Logger';

const MapboxGL = {...NativeModules.MGLModule};

const eventEmitter = new NativeEventEmitter(NativeModules.MGLModule);

MapboxGL.registerDenmCallback = (callback) => {
  eventEmitter.addListener('DENM', ({ geojson }) => {
    callback(geojson);
  });
}

MapboxGL.registerCpmCallback = (callback) => {
  eventEmitter.addListener('CPM', () => {
    callback();
  });
}

MapboxGL.registerMqttConnectionCallback = (callback) => {
  eventEmitter.addListener('MQTT_CONNECT', ({connectionState}) => {
    callback(connectionState);
  });
}

MapboxGL.registerGeojsonCallback = (callback) => {
  eventEmitter.addListener('GEOJSON', ({ type, geoJSON }) => {
    callback(type, geoJSON);
  });
}

MapboxGL.registerObuPosCallback = (callback) => {
  eventEmitter.addListener('OBU_INFO', ({ obuInfo }) => {
    callback(obuInfo);
  });
}

MapboxGL.registerGlosaCallback = (callback) => {
  eventEmitter.addListener('GLOSA', ({ tlSpatList }) => {
    callback(tlSpatList);
  });
}

MapboxGL.unregisterGeojsonCallback = () => {
  eventEmitter.removeAllListeners('GEOJSON');
}

MapboxGL.unregisterObuPosCallback = () => {
  eventEmitter.removeAllListeners('OBU_INFO');
}

MapboxGL.unregisterGlosaCallback = () => {
  eventEmitter.removeAllListeners('GLOSA');
}

MapboxGL.unregisterDenmCallback = () => {
  eventEmitter.removeAllListeners('DENM');
}

MapboxGL.unregisterCpmCallback = () => {
  eventEmitter.removeAllListeners('CPM');
}

MapboxGL.unregisterMqttConnectionCallback = () => {
  eventEmitter.removeAllListeners('MQTT_CONNECT');
}

MapboxGL.unregisterAllCallbacks = () => {
  eventEmitter.removeAllListeners('MQTT_CONNECT');
  eventEmitter.removeAllListeners('DENM');
  eventEmitter.removeAllListeners('CPM');
  eventEmitter.removeAllListeners('OBU_INFO');
  eventEmitter.removeAllListeners('GEOJSON');
  eventEmitter.removeAllListeners('GLOSA');
}
// static methods
MapboxGL.requestAndroidLocationPermissions = async function () {
  if (isAndroid()) {
    const res = await PermissionsAndroid.requestMultiple([
      PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
      PermissionsAndroid.PERMISSIONS.ACCESS_COARSE_LOCATION,
    ]);

    if (!res) {
      return false;
    }

    const permissions = Object.keys(res);
    for (const permission of permissions) {
      if (res[permission] === PermissionsAndroid.RESULTS.GRANTED) {
        return true;
      }
    }

    return false;
  }

  throw new Error('You should only call this method on Android!');
};

MapboxGL.UserTrackingModes = Camera.UserTrackingModes;

// components
MapboxGL.MapView = MapView;
MapboxGL.Light = Light;
MapboxGL.PointAnnotation = PointAnnotation;
MapboxGL.Callout = Callout;
MapboxGL.UserLocation = UserLocation;
MapboxGL.Camera = Camera;
MapboxGL.Style = Style;

// annotations
MapboxGL.Annotation = Annotation;
MapboxGL.MarkerView = MarkerView;

// sources
MapboxGL.VectorSource = VectorSource;
MapboxGL.ShapeSource = ShapeSource;
MapboxGL.RasterSource = RasterSource;
MapboxGL.ImageSource = ImageSource;
MapboxGL.Images = Images;

// layers
MapboxGL.FillLayer = FillLayer;
MapboxGL.FillExtrusionLayer = FillExtrusionLayer;
MapboxGL.HeatmapLayer = HeatmapLayer;
MapboxGL.LineLayer = LineLayer;
MapboxGL.CircleLayer = CircleLayer;
MapboxGL.SymbolLayer = SymbolLayer;
MapboxGL.RasterLayer = RasterLayer;
MapboxGL.BackgroundLayer = BackgroundLayer;

// modules
MapboxGL.locationManager = locationManager;
MapboxGL.offlineManager = offlineManager;
MapboxGL.snapshotManager = snapshotManager;

// animated
MapboxGL.Animated = Animated;

// utils
MapboxGL.AnimatedPoint = AnimatedMapPoint;
MapboxGL.AnimatedCoordinatesArray = AnimatedCoordinatesArray;
MapboxGL.AnimatedExtractCoordinateFromArray =
  AnimatedExtractCoordinateFromArray;
MapboxGL.AnimatedRouteCoordinatesArray = AnimatedRouteCoordinatesArray;
MapboxGL.AnimatedShape = AnimatedShape;
MapboxGL.Logger = Logger;

const {LineJoin} = MapboxGL;

export {
  MapView,
  Light,
  PointAnnotation,
  Callout,
  UserLocation,
  Camera,
  Annotation,
  MarkerView,
  VectorSource,
  ShapeSource,
  RasterSource,
  ImageSource,
  Images,
  FillLayer,
  FillExtrusionLayer,
  HeatmapLayer,
  LineLayer,
  CircleLayer,
  SymbolLayer,
  RasterLayer,
  BackgroundLayer,
  locationManager,
  offlineManager,
  snapshotManager,
  AnimatedMapPoint,
  AnimatedCoordinatesArray,
  AnimatedShape,
  Animated,
  LineJoin,
  Logger,
  Style,
};

export default MapboxGL;
