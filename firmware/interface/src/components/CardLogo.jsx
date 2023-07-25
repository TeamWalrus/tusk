import hidLogo from "../assets/hid_logo.png";

export default function CardLogo({ cardType }) {
  switch (cardType) {
    case "hid":
      return (
        <img
          className="mx-auto h-auto max-w-xs"
          width="60px"
          src={hidLogo}
          alt="hid-logo"
        />
      );
  }
}
