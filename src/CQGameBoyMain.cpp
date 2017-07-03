#include <CQGameBoyScreen.h>
#include <CQGameBoyVideo.h>
#include <CQGameBoyKeys.h>
#include <CQGameBoyDbg.h>
#include <CQGameBoy.h>
#include <CZ80RstData.h>
#include <CArgs.h>
#include <CQApp.h>

int
main(int argc, char **argv)
{
  CQApp app(argc, argv);

  CArgs cargs("-v:f                (verbose) "
              "-dump:f             (enable dump) "
              "-invert:f           (invert screen colors) "
              "-scale:i=1          (scale factor) "
              "-debug:f            (debug) "
              "-graphics:f         (show graphics) "
              "-graphics_scale:i=1 (graphics scale factor) "
              "-keys:f             (show keys) "
              "-asm:f              (load assembly) "
              "-test:f             (add test support routines) "
              );

  cargs.parse(&argc, argv);

  bool verbose        = cargs.getBooleanArg("-v");
  bool dump           = cargs.getBooleanArg("-dump");
  bool invert         = cargs.getBooleanArg("-invert");
  int  scale          = cargs.getIntegerArg("-scale");
  bool debug          = cargs.getBooleanArg("-debug");
  bool graphics       = cargs.getBooleanArg("-graphics");
  int  graphics_scale = cargs.getIntegerArg("-graphics_scale");
  bool keys           = cargs.getBooleanArg("-keys");
  bool assembly       = cargs.getBooleanArg("-asm");
  bool test           = cargs.getBooleanArg("-test");

  CQGameBoy *gameboy = new CQGameBoy;

  CZ80 *z80 = gameboy->getZ80();

  z80->setVerbose(verbose);
  z80->setDump(dump);

  gameboy->setInvert(invert);
  gameboy->setScale (scale );

  gameboy->createScreen();

  //------

  QFont fixedFont("Courier New", 16);

  gameboy->setFixedFont(fixedFont);

  //------

  if (graphics) {
    CQGameBoyVideo *video = gameboy->addVideo();

    video->setScale(graphics_scale);
  }

  //------

  if (keys)
    gameboy->addKeys();

  //------

  CZ80StdRstData rst_data(*z80);

  if (test)
    z80->setRstData(&rst_data);

  //------

  for (int i = 1; i < argc; ++i) {
    if (assembly)
      gameboy->loadAsm(argv[i]);
    else
      gameboy->loadCartridge(argv[i]);
  }

  CQGameBoyScreen *screen = gameboy->screen();

  z80->setScreen(screen);

  screen->show();

  //------

  if (debug)
    gameboy->addDebug();

  //------

  if (! debug && ! graphics)
    screen->exec();

  return app.exec();
}
