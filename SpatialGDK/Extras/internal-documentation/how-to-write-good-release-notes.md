# Guide: writing good release notes

* [Guiding principles](#guiding-principles)
* [Helpful stuff](#helpful-stuff)
  * [Writing techniques](#writing-techniques)
  * [Templates](#templates)
  * [Examples: improving your release notes](#examples-improving-your-release-notes)
  * [Examples of good release notes](#examples-of-good-release-notes)
* [Adding release notes](#adding-release-notes)
  * [About the release notes tool](#about-the-release-notes-tool)
  * [How to...](#how-to)
  * [Directory structure and relevant files](#directory-structure-and-relevant-files)

Release notes are a chance for us to show off the new stuff we've done, and also demonstrate
that we're actively fixing bugs and improving things.

We point users towards them when we release a new version. They might be waiting for a particular
bugfix, or a particular feature we've mentioned. **If the release notes are clear and consistent,
it's easier for users to skim them and find what they're looking for.**

## Guiding principles
* Release notes should be accurate, clear, and concise. If you can't tick all of these off, the
order of priority is **accurate** > **clear** > **concise**.
* **WIIFM - what's in it for me?** ie: why should users care? A release note shouldn't be like
a stand-up update, focusing on the details of what you've changed in the code. Tell users
explicitly how it affects them.
* **Give enough context.** Tell users what they can/should do next. Make sure there's enough
detail for users to do something with the information. Link to docs when they're helpful. 
* **You don't need to start bugfixes with "Fixed...".** It often makes the sentence more
convoluted and harder to read. Try one of the [suggested formats](#templates) instead.
* **You don't need to be formal and impersonal.** Speak directly to the user ("You can...") and
personify Improbable ("We've updated...").
* **Review your writing.** Read it through and think about whether you could make it clearer or 
more helpful. You could also run it through [Hemingway App](http://www.hemingwayapp.com/), which 
highlights your writing if you've used the passive, or if you've used a phrase that has a 
simpler alternative. It also highlights sentences that are hard to read.
* **Use brain.** Regardless of all this: if your release note sounds weird or unnatural,
change it.

## Helpful stuff
### Writing techniques
* **Expand and then simplify**. Like when you're solving an equation - you wouldn’t just look
at it and expect the answer to materialise. It can help to write the information out in full
and then condense it down once you've got it all in front of you.
* **Imagine you're explaining it to someone**. Write it down in a conversational way, even if
it’s much longer than you want it to be eventually. 
* **Don't be afraid to split a release note into a couple of sentences**, rather than trying
to make it work as one long sentence. And you can always link to another page for more information.
* Sometimes it's useful to say what the situation was previously, and how it's different now.

### Templates

* _You can now..._

  > You can now de-register the same event callback of an obsolete Unity Reader or Writer
  more than once.

* _X now/no longer does Y when Z._

  > A `GameObject` with the same name as another asset type will no longer clash and cause
  problems when spawning new entities.

* _X now/no longer does Y. This means you no longer/now need to Z._

  > The C++ SDK now automatically sends built-in metrics to SpatialOS. This means you no
  longer need to manually send these metrics when they are surfaced via `Dispatcher::OnMetrics`.

* _We've improved/increased..._

  > We’ve increased the timeout for receiving initial worker flags. This works around a
  problem where workers might occasionally time out when using a large snapshot.

### Examples: improving your release notes

* _Formal and impersonal -> more direct_

  **First draft**:
  > It is now possible to directly use `improbable.Coordinates`, `improbable.Vector3d` and
  `improbable.Vector3f` as return types for commands. It is not necessary anymore to wrap
  them in a user-defined type.

  **Improved version**:
  > You can now directly use `improbable.Coordinates`, `improbable.Vector3d` and
  `improbable.Vector3f` as return types for commands. You no longer need to wrap them in
  a user-defined type.

* _Concise but unclear -> longer but clearer -> concise and clear_

  **First draft**:
  > Previous unresponded command responder will not be lost when a new command is received
  before it is sent.

  **Expanded out**:
  > Before this fix, a component command could fail when the component received a new command
  before sending the response to the previous command. Now, no unresponded commands will fail,
  no matter how many new commands are received before responding.

  **Condensed down and improved**:
  > Component commands no longer fail if the component receives a new command before it has
  sent the response to the previous command.

* _"Stand-up"-style -> more user-focused (WIIFM)_

  **First draft**:
  > Added the UsersConnected and UsersCapacity fields to the `worker::Deployment` struct
  returned by `worker::Locator::GetDeploymentListAsync()`.

  **Improved version**:
  >You can now see how many users are connected to a deployment (`UsersConnected`), and the
  user capacity of the deployment (`UsersCapacity`), using `improbable.worker.Locator.getDeploymentListAsync()`.

### Examples of good release notes

* Added `FRequestId` as the return value of every command. This can be used to link up and
trace the sent request.

* When you’re using `EntityBuilder`, you can now set write access on the `EntityACL` component
itself using `.SetEntityAclComponentWriteAccess()`. For details, see _Creating and deleting
entities_.

* Blueprints and generated code for `EntityId` now use the new wrapper type `FEntityId` instead
of `int`.

* The C# SDK no longer depends on protobuf-net, and protobuf-net is no longer distributed with
the SDK. If you have customised project files for your C# workers, you will need to update them
to remove this dependency.

* The `PrefabName` and `PrefabContext` fields have been removed from `AddEntityOp`. If you need
to set a prefab name, use the `Improbable.Metadata` standard component.

## Adding release notes

You add release notes in the `##Unreleased` section of [CHANGELOG.md](../../../CHANGELOG.md).