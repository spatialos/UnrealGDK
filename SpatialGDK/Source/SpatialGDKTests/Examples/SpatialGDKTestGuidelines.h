/*
1. How to run tests:
	- Tests can be run via Session Frontend in UE4 Editor
	(https://docs.unrealengine.com/en-US/Programming/Automation/index.html has more information)

	- Tests can be run via command line:
	{PathToUnreal}\Engine\Binaries\Win64\UE4Editor-Cmd.exe ^
	 "{PathToUProjectFile}.uproject" ^
	 -unattended -nopause -NullRHI -log -log=RunTests.log ^
	 -ExecCmds="Automation RunTests {TestFilter}; Quit"

	where {TestFilter} is a match on the test names
	(e.g SpatialGDK matches all GDK tests, FRPCContainer matches all tests in RPCContainerTest)

2. Folder structure
	- Testing code that is not a part of exposed API
		- These tests should go into the Private\Tests directory within the relevant module.
		When an Automation Test matches one-to-one with a particular class, the test file should be named [ClassFilename]Test.cpp,
		e.g. a test that applies only to FText would be written in TextTest.cpp.

	- Testing code that is a part of exposed API or Integration tests
		- These tests are located in a separate SpatialGDKTests module.
		So for every component that needs to be tested - it has to be exposed via corresponding macro.
		E.g. `class FRPCContainer` -> `class SPATIALGDK_API FRPCContainer`, to make FRPCContainer testable.
		The macro is different in each module SPATIALGDK_API, SPATIALGDKSERVICES_API etc.

		- The folder structure inside SpatialGDKTests should resemble the folder structure for components being tested.
		E.g. If file tested is
		`UnrealGDK\SpatialGDK\Source\SpatialGDK\Public\Utils\RPCContainer.cpp`,
		then the corresponding test should be located in
		`UnrealGDK\SpatialGDK\Source\SpatialGDKTests\SpatialGDK\Utils\RPCContainer\RPCContainerTest.cpp`.
		There should be a folder with the same name as component tested. It should include the test and all the supporting files.

		- In case of integration tests, that involve multiple components - they should be located in
		`UnrealGDK\SpatialGDK\Source\SpatialGDKTests\IntegrationTests\<Custom Test Folder>` folder.

		- Tests should be stripped out in Shipping builds.
		If tests are not in SpatialGDKTests module - their code should be surrounded with `#if !UE_BUILD_SHIPPING` macro
		SpatialGDKTests module should be excluded in shipping builds.

3. Test definitions (check TestDefinitions.h for more info)
	- We have defined 3 types of Macro to be used when writing tests:
	(https://docs.unrealengine.com/en-US/Programming/Automation/TechnicalGuide/index.html has more information)
	GDK_TEST - a simple test, that should be used if it's one of a kind (i.e. it's body can't be reused, otherwise use GDK_COMPLEX_TEST),
	and if doesn't rely on background threads doing the computation (otherwise use LATENT_COMMANDs).
	GDK_COMPLEX_TEST - same as simple test, but allows having multiple test cases run through the same test function body.
	DEFINE_LATENT_COMMAND... - used to run tests that are expected to run across multiple ticks.
	Latent command names should start with `F` (e.g. DEFINE_LATENT_AUTOMATION_COMMAND(FStartDeployment)").

	- There are 5 types of mock objects we can use in tests:
	Dummy objects
		are passed around but never actually used. Usually they are just used to fill parameter lists.

	Fake objects
		actually have working implementations, but usually take some shortcut which makes them not suitable for production (an
InMemoryTestDatabase is a good example).

	Stubs
		provide canned answers to calls made during the test, usually not responding at all to anything outside what's programmed in for the
test.

	Spies
		are stubs that also record some information based on how they were called.
		One form of this might be an email service that records how many messages it was sent.

	Mocks
		are pre-programmed with expectations which form a specification of the calls they are expected to receive.
		They can throw an exception if they receive a call they don't expect and are checked during verification to ensure they got all the
calls they were expecting.

4. Test naming convention
	- `GIVEN_WHEN_THEN` should be used.
	E.g. `GIVEN_one_and_two_WHEN_summed_THEN_the_sum_is_three`

5. Test coverage
	- Unit tests should be isolated: Tests should be runnable on any machine, in any order, without affecting each other.
	If possible, tests should have no dependencies on environmental factors or global/external state.
	- Unit tests should verify a single use-case or code path:
	they are simpler and more understandable, and that is good for maintainability and debugging.
	- Unit tests should use a minimal (1, when possible) number of TestTrue/TestFalse assertions,
	that covers only what is needed for the use-case/code path you are testing.

6. Test fixtures (to perform the setup and cleanup before and after test correspondingly)
	- There are no Test Fixtures out of the box in Unreal Automation Testing Framework
	The solution for now is to instantiate an object at the beginning of test,
	that sets up the environment in the constructor and cleans it up in the destructor
	(however, that cannot be used with Latent Commands,
	since the destructor will likely to be called earlier than necessary).
*/
