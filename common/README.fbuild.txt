To build projects using FASTBuild you need to configure build environment
first by creating local.bff. There are three ways of doing this:
   1. Use local.bff.sample as a base to write your own. You will probably only
   need to specify some paths to libs.
   2. (Windows-only) Copy local.bff.compat to local.bff. This will use old
   environment variables (like BOOST_DIR).
   3. Use cmake to configure build and create local.bff
