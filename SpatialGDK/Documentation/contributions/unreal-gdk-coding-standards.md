
<%(TOC)%>
# GDK for Unreal C++ coding standards

> In general, we follow the [Coding Standard](https://docs.unrealengine.com/en-us/Programming/Development/CodingStandard) (Unreal documentation) set out by Epic. This page highlights some additions to Epic's guidelines.

* Avoid assignments in if statements unless the variable is declared in the statement.

    It's good practice to limit the lifetime of a variable using the scope. For instance if a variable is only used within an if statement, then its lifetime is cleanly managed using the following approach:

        if (APawn* Pawn = Cast<APawn>(Actor))
        {
            //do something with the Pawn variable
        }

    This is safe as the compiler will generate a C2143 compiler error if a comparison is accidently added:

        if (APawn* Pawn == Cast<APawn>(Actor))
        {
            // will generate a C2143
        }

    However, if the variable is declared earlier in the outside scope and cannot be contained within the scope of the statement, then we should avoid using assignments in the statement:

    For example, prefer:

        APawn* Pawn = Cast<APawn>(Actor);
        if (Pawn != nullptr)
        {
            //do something with the Pawn variable
        }

    to:

        APawn* Pawn;

        if(Pawn = Cast<APawn>(Actor)) //NOOOO
        {

        }

* Wrap native C++ classes/structs in the improbable namespace
    For any object that is not tagged with USTRUCT or UCLASS, we should use the namespace `improbable` to avoid name collision. The classes that are wrapped in namespaces should not use the U,F or A prefixes to make it easy to distiguish these from the Unreal classes.
