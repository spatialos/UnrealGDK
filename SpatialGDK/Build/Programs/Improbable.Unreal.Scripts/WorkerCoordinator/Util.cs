// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using System;
using System.Collections.Generic;
using System.Linq;

namespace Improbable.WorkerCoordinator
{
    internal static class Util
    {
        /// <summary>
        /// Get whether the specified argument is given in the list of arguments.
        /// Looks for arguments in the form {argumentName}={value}.
        /// </summary>
        public static bool HasIntegerArgument(IEnumerable<string> args, string argumentName)
        {
            return args.Where(arg => arg.StartsWith($"{argumentName}=")).Count() > 0;
        }

        /// <summary>
        /// Get the value of the named argument in the given list of arguments.
        /// Looks for arguments in the from {argumentName}={value}.
        ///
        /// Throws an exception if none or more than 1 occurrences of the argument are found,
        /// or the argument value is formatted incorrectly.
        /// </summary>
        public static int GetIntegerArgument(IEnumerable<string> args, string argumentName)
        {
            var argsWithName = args.Where(arg => arg.StartsWith($"{argumentName}=")).ToArray();
            if (argsWithName.Length != 1)
            {
                throw new ArgumentException($"Expected exactly one value for argument \"{argumentName}\", found {argsWithName.Length}.");
            }

            var argWithName = argsWithName[0];
            var split = argWithName.Split(new[] { '=' }, 2, StringSplitOptions.None);
            if (split.Length != 2)
            {
                throw new ArgumentException($"Cannot parse value for argument \"{argumentName}\". Expected format \"{argumentName}=<integer>\", found \"{argWithName}\".");
            }

            var valueString = split[1];
            if (int.TryParse(valueString, out int value))
            {
                return value;
            }

            throw new ArgumentException($"Cannot parse value,\"{valueString}\", for argument \"{argumentName}\".");
        }

        /// <summary>
        /// Get the value of the named argument in the given list of arguments.
        /// Looks for arguments in the from {argumentName}={value}.
        ///
        /// If the argument is not present in the list of arguments, the default value is returned.
        ///
        /// Throws an exception if more than 1 occurrences of the argument are found,
        /// or the argument value is formatted incorrectly.
        /// </summary>
        public static int GetIntegerArgumentOrDefault(IEnumerable<string> args, string argumentName, int defaultValue)
        {
            if (HasIntegerArgument(args, argumentName))
            {
                return GetIntegerArgument(args, argumentName);
            }

            return defaultValue;
        }

        /// <summary>
        /// Replaces each argument in args that is present as a key in argumentReplacements with the corresponding value in argumentReplacements.
        /// Does not modify the input arguments, returns a new array with the replaced arguments.
        /// </summary>
        /// <param name="args">array of arguments to check for placeholders to replace with a value</param>
        /// <param name="argumentReplacements">mapping containing the arguments to replace as keys and values the replacement values</param>
        /// <returns>copy of input args with placeholder arguments replaced with their values</returns>
        public static string[] ReplacePlaceholderArgs(string[] args, Dictionary<string, string> argumentReplacements)
        {
            return args.ToList().ConvertAll(arg =>
            {
                if (argumentReplacements.ContainsKey(arg))
                {
                    return argumentReplacements[arg];
                }

                return arg;
            }).ToArray();
        }
    }
}
