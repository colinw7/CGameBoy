#include <CQGameBoy.h>
#include <CQGameBoyGraphics.h>
#include <CQGameBoyDbg.h>
#include <CGameBoy.h>
#include <CArgs.h>
#include <CQApp.h>

int
main(int argc, char **argv)
{
  CQApp app(argc, argv);

  CGameBoy *gameboy = new CGameBoy;

  CArgs cargs("-v:f                (verbose) "
              "-dump:f             (enable dump) "
              "-invert:f           (invert screen colors) "
              "-scale:i=1          (scale factor) "
              "-debug:f            (debug) "
              "-graphics:f         (show graphics) "
              "-graphics_scale:i=1 (graphics scale factor) "
              );

  cargs.parse(&argc, argv);

  bool verbose        = cargs.getBooleanArg("-v");
  bool dump           = cargs.getBooleanArg("-dump");
  bool invert         = cargs.getBooleanArg("-invert");
  int  scale          = cargs.getIntegerArg("-scale");
  bool debug          = cargs.getBooleanArg("-debug");
  bool graphics       = cargs.getBooleanArg("-graphics");
  int  graphics_scale = cargs.getIntegerArg("-graphics_scale");

  CZ80 *z80 = gameboy->getZ80();

  z80->setVerbose(verbose);
  z80->setDump(dump);

  //z80->setMemFlags(0xff44, 1, CZ80MemType::SCREEN);

  gameboy->setInvert(invert);
  gameboy->setScale (scale );

  //------

  QFont fixedFont("Courier New", 16);

  //------

  CQGameBoy *qgameboy = new CQGameBoy(gameboy);

  if (graphics) {
    CQGameBoyVideo *videoWindow = new CQGameBoyVideo(qgameboy);

    videoWindow->setScale(graphics_scale);

    videoWindow->setFixedFont(fixedFont);

    videoWindow->show();
  }

  //------

  for (int i = 1; i < argc; ++i)
    gameboy->loadCartridge(argv[i]);

  z80->setScreen(qgameboy);

  qgameboy->show();

  if (debug) {
    CQGameBoyDbg *dbg = new CQGameBoyDbg(qgameboy);

    dbg->init();

    dbg->setFixedFont(fixedFont);

    dbg->show();
  }

  if (! debug && ! graphics)
    qgameboy->exec();

  return app.exec();
}
