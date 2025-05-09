Hi there, thank you for looking at this project.

There are quite a few things bundled here, the main parts being: automation.py, main project, tests and reference project.

automation.py is a script which is used to test the main program under different genetic configurations.
You can alter these by adding, removing or changing the values stored in the PARAM_RANGES list within the script.
There are comments indicating which values correspond to which program parameters. 
Upon invoking the script, it will change the parameters, recompile the project, run the new instance of the project and 
record it's stdout to a file in sim_outputs.


In the tests you can ask the project to find the best configuration iteratively, it will do this based on current settings,
although it would not be recommended to run it when the data width is large. 

Within the main project, all the important configuration settings are stored and managed within Types.hpp. 
To manually configure the genetic parameters, comment out the 
"inline constexpr double INITIAL_TEMPERATURE = SCRIPT_GENERATED_VALUE_1;" block of code and uncomment the block above it.
This will allow you to manually set the parameters. 
As a user, it would not be recommended to change anything outside of 
the Constants::General:: or Constants::Genetic:: name spaces. 

The reference project pertains to the files under src/Old/. It is currently only known to work on Linux systems although
it might be possible to run with some subsystem install on windows. 