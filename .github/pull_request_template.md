<!-- Provide a general summary of your changes in the Title above -->
<!-- It must be meaningful and coherent with the changes -->

<!--
    If this PR is still a Work in Progress [WIP], please open it as DRAFT.
    Please consider if any label should be added to this PR.
    If no code has been changed, please add `skip-ci` label.
    If opening the PR as Draft, please consider adding `no-test` label to only build the code but not run CI.
    If documentation PR is still pending, please add `doc-pending` label.
-->

## Description

<!--
    Describe changes in detail.
    This includes depicting the context, use case or current behavior and describe the proposed changes.
    If several features/bug fixes are included with these changes, please consider opening separated pull requests.
-->

<!--
    In case of bug fixes, please provide the list of supported branches where this fix should be also merged.
    Please uncomment following line, adjusting the corresponding target branches for the backport.
-->
<!-- @Mergifyio backport 3.1.x 2.14.x 2.10.x -->

<!--
    In case of critical bug fix, please uncomment following line, adjusting the corresponding LTS target branches for the backport.
-->
<!-- @Mergifyio backport 2.6.x -->

<!-- If an issue is already opened, please uncomment next line with the corresponding issue number. -->
<!-- Fixes #(issue) -->

<!-- In case the changes are built over a previous pull request, please uncomment next line. -->
<!-- This PR depends on #(PR) and must be merged after that one. -->

## Contributor Checklist

<!--
    - If any of the elements of the following checklist is not applicable, substitute the checkbox [ ] by _N/A_:
    - If any of the elements of the following checklist is not fulfilled on purpose, please provide a reason and substitute the checkbox [ ] with ❌: or __NO__:.
-->

- [ ] Commit messages follow the project guidelines. <!-- External contributors should sign the DCO. Fast DDS developers must also refer to the internal Redmine task. -->
- [ ] The code follows the style guidelines of this project. <!-- Please refer to the [Quality Declaration](https://github.com/eProsima/Fast-DDS/blob/master/QUALITY.md#linters-and-static-analysis-4v) for more information. -->
- [ ] Tests that thoroughly check the new feature have been added/Regression tests checking the bug and its fix have been added; the added tests pass locally <!-- Blackbox tests checking the new functionality are required. Changes that add/modify public API must include unit tests covering all possible cases. In case that no tests are provided, please justify why. -->
- [ ] Any new/modified methods have been properly documented using Doxygen. <!-- Even internal classes, and private methods and members should be documented, not only the public API. -->
- [ ] Any new configuration API has an equivalent XML API (with the corresponding XSD extension) <!-- C++ configurable parameters should also be configurable using XML files. -->
- [ ] Changes are backport compatible: they do **NOT** break ABI nor change library core behavior. <!-- Bug fixes should be ABI compatible if possible so a backport to previous affected releases can be made. -->
- [ ] Changes are API compatible. <!-- Public API must not be broken within the same major release. -->
- [ ] New feature has been added to the `versions.md` file (if applicable).
- [ ] New feature has been documented/Current behavior is correctly described in the documentation. <!-- Please uncomment following line with the corresponding PR to the documentation project: -->
    <!-- - Related documentation PR: eProsima/Fast-DDS-docs#(PR) -->
- [ ] Applicable backports have been included in the description.

## Reviewer Checklist

- [ ] The PR has a milestone assigned.
- [ ] The title and description correctly express the PR's purpose.
- [ ] Check contributor checklist is correct.
- [ ] If this is a critical bug fix, backports to the critical-only supported branches have been requested.
- [ ] Check CI results: changes do not issue any warning.
- [ ] Check CI results: failing tests are unrelated with the changes.
