# Readme

## Divergences from standard (rationales)

### value-types instance of reference-types
This implementation of xtypes traits the DynamicType objects as value-types.
This could be created any copy (little copies) but gives to the user a more safety API.
For instance, future modifications of a DynamicType already in a Publisher/Subscriber do not broke the Publisher/Subscriber.
Also, the type definition is in configuration stage, where the performance is not significant.
RTI uses this architecture.

### LoanValue without lock
The internal implementation of DynamicTypes does not need to lokc/unlock the loan values.
This allow to simplify the API access of nested members in DynamicData

### Removed Annotation clases
Annotation concepts has been embebed into the places where they are necessary in a more intuitive way.

### Added some methods out of standard to ease the live to the user
[] operator to access members.
DynamicTypeIterator class

