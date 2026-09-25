# Security Policy

*Last updated: 2026-09-25*

## Motivation

As a proud member of the open source community, eProsima takes the security of its software very seriously.
As such, we would like to be informed when a security bug is found so that it can be fixed and disclosed as quickly as possible.
The rest of the document outlines what is covered by this policy and how to report security vulnerabilities.

This policy applies to eProsima Fast DDS.
For this repository it replaces the [organization-wide security policy](https://github.com/eProsima/.github/blob/main/SECURITY.md); where the two differ, the terms below apply.

## Scope

The Vulnerability Disclosure Program outlined here covers the code in this repository and the eProsima Fast DDS releases built from it.
An increasing number of companies are using our DDS based solutions, and during this growth period we anticipate vulnerabilities to be identified in products before vendors have an established vulnerability reporting program.
We will work with you on a best-effort basis to help connect you with responsible parties best suited to address your concerns.

## Out of scope

This program covers the source code and released artifacts described above.
eProsima's web sites, cloud services and corporate infrastructure, including eprosima.com, are not part of this program, and this policy does not authorize any testing against them.

Our software is a component of a larger system, and its security depends on the environment in which that system runs.
Issues that require an attacker to already hold the privileges of the application process, such as the ability to execute code within it, to read or modify its memory, or to alter its configuration, credentials or key material on disk, are not considered vulnerabilities in our products.
Ensuring the security of the host and of the processes that use our libraries is the responsibility of the integrator.
This includes the configuration of our software: where a setting widens access to a resource beyond its default, such as the permissions of a shared memory segment, the consequences of that choice are out of scope.

This does not exclude defects in our software itself, such as a resource being created with weaker permissions than its documented or configured value.
Reports of that kind are in scope.

We accept reports against the release branches listed as currently supported in [RELEASE_SUPPORT.md](https://github.com/eProsima/Fast-DDS/blob/master/RELEASE_SUPPORT.md), and against the development branch.
Long Term Supported branches keep receiving security fixes after their standard support ends, until the end of life (EOL) date given in that document.
Branches that have reached EOL are out of scope.
If you find an issue on an EOL branch, please check whether it reproduces on a supported branch before reporting it; if it does not, the remedy is to upgrade.

The following activities are not authorized under this policy, and therefore fall outside the Safe Harbor described below:

- Denial of service, resource exhaustion, or any other form of stress testing.
- Social engineering or phishing directed at eProsima personnel, users or customers.
- Physical access attempts against eProsima offices, staff or equipment.
- Accessing, modifying or extracting data belonging to other users, customers or third parties.
- Automated scanning or fuzzing directed at production systems or services operated by eProsima or by its customers.

None of the above restricts testing against your own builds and deployments of our software, which is the kind of research this program is intended to encourage.
If your research would require any of the activities listed above, please contact us at support@eprosima.com before proceeding.

## Safe Harbor

eProsima strongly supports security research into its software and seeks to encourage that research.
eProsima will not engage in legal action against individuals who act in good faith to identify, report and fix vulnerabilities in our products, so long as they operate in accordance with any applicable laws and this policy.
Research or testing against DDS systems without the consent of the owner/operator is in violation of this policy and strongly discouraged due to potential health and human safety concerns.

This policy may be revised from time to time.
The version of this policy in effect at the time your research is carried out is the version that applies to it; previous versions remain available in this file's [commit history](https://github.com/eProsima/Fast-DDS/commits/master/SECURITY.md).

If at any time you have concerns about whether your activities are consistent with this policy, please contact us at support@eprosima.com.

## How to submit a vulnerability

Please report security vulnerabilities using GitHub Security Advisories whenever possible.

Open a private report with the [**Report a vulnerability**](https://github.com/eProsima/Fast-DDS/security/advisories/new) form for this repository.
The same form is reachable from the **Security** tab of the repository.
This creates a private security advisory that can be used to discuss, triage, and resolve the vulnerability without disclosing sensitive information publicly.
Please state which Fast DDS versions are affected.

If you are unable to use GitHub Security Advisories, you may instead submit the vulnerability by emailing support@eprosima.com.
In that case, we kindly ask you to encrypt your report using the PGP public key [vulnerability-public.key](https://github.com/eProsima/policies/blob/main/vulnerability-public.key) contained in the [eProsima/policies](https://github.com/eProsima/policies) repository.
Before encrypting, please verify that the key fingerprint is `47C9 326F B336 F759 7EB4 809E AF88 37E2 B6A8 41B6`.

Whichever reporting mechanism you use, the more information provided about the bug, the easier it will be to investigate and fix.
If you already have a fix, please include it with your report.

From there, we may set up a secure channel to discuss sensitive details about the vulnerability which may include proof-of-concept code, an impact assessment, recommended remediation steps or other information that will help us find and fix the problem.

Include any plans you may have to disclose details about the vulnerability.
In order to protect our users, unless otherwise agreed by both parties, we ask that you not publicly discuss the vulnerability until a fix is published or at least 90 days have passed since the initial report submission.
The group handling the report will also follow this timeline.
If you do not wish to be acknowledged in the release communications please indicate so when you submit the vulnerability.

## Use of AI assistance in reports

You may use AI tools to help you find, analyze or describe a vulnerability.
If you do, you remain fully responsible for the content of your report, exactly as if you had written every part of it yourself.
We also ask that you tell us which parts of the report were produced with AI assistance.

Before submitting, please confirm that you have reproduced the issue against the affected version of our software, that the functions, files and code paths your report refers to actually exist, and that its conclusions still hold when you check them yourself.

Please describe the issue briefly and in your own words.
A long generated explanation takes far longer to read than it took to produce, and that cost falls on the small team that has to triage it.
The few extra minutes you spend making a report short and specific are worth more to us than any volume of generated text.

Please do not use automated agents or bots to open security advisories in our repositories.
Reports submitted that way tend to arrive in duplicate, to describe behaviour that is not present in the code, or to restate findings that are already public, and triaging them takes time away from fixing real vulnerabilities.
Every report should be reviewed and submitted by a person who has verified it.

Reports that appear to be unverified machine output will be closed without a detailed analysis.
Repeatedly submitting such reports may lead us to block the account or address they come from.

## What to expect in response to a vulnerability disclosure

Expect a timely response to your notification, normally within two business days, during which time we will triage your vulnerability report.

After we have evaluated the vulnerability we will send you our risk assessment and, where applicable, the anticipated time to publish an update.
We will aim at being as transparent as possible about timelines and any issues or challenges which may impact the scheduled fix.

We will notify you when a pull request has been submitted and approved which remediates the vulnerability.
Because security vulnerabilities can have severe consequences, they differ from other bugs and at our discretion.
In order to protect our users we may modify our normal code release processes for publishing open source code updates.
