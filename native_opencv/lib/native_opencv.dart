import 'dart:ffi';
import 'dart:typed_data';
import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';

final DynamicLibrary opencvLibrary = DynamicLibrary.open("libnative_opencv.so");

typedef ReceiveImageArrayCallback = Void Function(Pointer<Uint8>, Int32);

class NativeOpencv {
  static const MethodChannel _channel = MethodChannel('native_opencv');

  static Future<void> processImage(
      String imagePath, void Function(Uint8List data) callback) async {
    final Pointer<Utf8> imagePathUtf8 = imagePath.toNativeUtf8();
    _userCallback = callback;
    try {
      _processImage(
          imagePathUtf8,
          Pointer.fromFunction<ReceiveImageArrayCallback>(
                  _receiveImageArrayCallback)
              .cast());
    } finally {
      calloc.free(imagePathUtf8);
    }
  }

  static void _receiveImageArrayCallback(Pointer<Uint8> data, int length) {
    final result = Uint8List.fromList(data.asTypedList(length));
    print(result);
    print('call back ==================================');

    _userCallback(result);
  }

  static void Function(Uint8List data) _userCallback = (data) {};

  static final _processImage = opencvLibrary.lookupFunction<
          Void Function(Pointer<Utf8> imagePath,
              Pointer<NativeFunction<ReceiveImageArrayCallback>> callback),
          void Function(Pointer<Utf8> imagePath,
              Pointer<NativeFunction<ReceiveImageArrayCallback>> callback)>(
      'processImage');
}
